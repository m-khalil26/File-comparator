//  Partie implantation du module holdall.

#include "holdall.h"

//  struct holdall, holdall : implantation par liste dynamique simplement
//    chainée. L'insertion a lieu en queue si la macroconstante
//    HOLDALL_INSERT_TAIL est définie, en tête sinon.

typedef struct choldall choldall;

struct choldall {
  void *value;
  choldall *next;
};

struct holdall {
  choldall *head;
#ifdef HOLDALL_INSERT_TAIL
  choldall *tail;
#endif
  size_t count;
};

holdall *holdall_empty(void) {
  holdall *ha = malloc(sizeof *ha);
  if (ha == NULL) {
    return NULL;
  }
  ha->head = NULL;
#ifdef HOLDALL_INSERT_TAIL
  ha->tail = NULL;
#endif
  ha->count = 0;
  return ha;
}

int holdall_put(holdall *ha, void *ptr) {
  choldall *p = malloc(sizeof *p);
  if (p == NULL) {
    return -1;
  }
  p->value = ptr;
#ifdef HOLDALL_INSERT_TAIL
  p->next = NULL;
  if (ha->tail == NULL) {
    ha->head = p;
  } else {
    ha->tail->next = p;
  }
  ha->tail = p;
#else
  p->next = ha->head;
  ha->head = p;
#endif
  ha->count += 1;
  return 0;
}

size_t holdall_count(holdall *ha) {
  return ha->count;
}

int holdall_apply(holdall *ha, int (*fun)(void *)) {
  for (const choldall *p = ha->head; p != NULL; p = p->next) {
    int r = fun(p->value);
    if (r != 0) {
      return r;
    }
  }
  return 0;
}

int holdall_apply_context(holdall *ha,
    void *context, void *(*fun1)(void *context, void *ptr),
    int (*fun2)(void *ptr, void *resultfun1)) {
  for (const choldall *p = ha->head; p != NULL; p = p->next) {
    int r = fun2(p->value, fun1(context, p->value));
    if (r != 0) {
      return r;
    }
  }
  return 0;
}

int holdall_apply_context2(holdall *ha,
    void *context1, void *(*fun1)(void *context1, void *ptr),
    void *context2, int (*fun2)(void *context2, void *ptr, void *resultfun1)) {
  for (const choldall *p = ha->head; p != NULL; p = p->next) {
    int r = fun2(context2, p->value, fun1(context1, p->value));
    if (r != 0) {
      return r;
    }
  }
  return 0;
}

void holdall_dispose(holdall **haptr) {
  if (*haptr == NULL) {
    return;
  }
  choldall *p = (*haptr)->head;
  while (p != NULL) {
    choldall *t = p;
    p = p->next;
    free(t);
  }
  free(*haptr);
  *haptr = NULL;
}

choldall *holdall_partition(choldall *head, choldall *tail, choldall **new_head,
    choldall **new_tail, int (*compar)(const void *,
    const void *)) {
  choldall *pivot = tail;
  choldall *prev = NULL;
  choldall *curr = head;
  tail = pivot;
  while (curr != pivot) {
    if (compar(curr, pivot) < 0) {
      if (*new_head == NULL) {
        *new_head = curr;
      }
      prev = curr;
      curr = curr->next;
    } else {
      if (prev != NULL) {
        prev->next = curr->next;
      }
      choldall *p = curr->next;
      curr->next = NULL;
      tail->next = curr;
      tail = curr;
      curr = p;
    }
  }
  if ((*new_head) == NULL) {
    *new_head = pivot;
  }
  *new_tail = tail;
  return pivot;
}

choldall *quick_sort(choldall *head, choldall *tail, int (*compar)(const void *,
    const void *)) {
  if (head == NULL || head == tail) {
    return head;
  }
  choldall *new_head = NULL;
  choldall *new_tail = NULL;
  choldall *pivot = holdall_partition(head, tail, &new_head, &new_tail, compar);
  if (new_head != pivot) {
    choldall *p = new_head;
    while (p->next != pivot) {
      p = p->next;
    }
    p->next = NULL;
    new_head = quick_sort(new_head, p, compar);
    p = new_head;
    while (p->next != NULL) {
      p = p->next;
    }
    p->next = pivot;
  }
  pivot->next = quick_sort(pivot->next, new_tail, compar);
  return new_head;
}

int holdall_sort(holdall *ha, int (*compar)(const void *, const void *)) {
  if (ha == NULL) {
    return -1;
  }
  choldall *p = ha->head;
  while (p->next != NULL) {
    p = p->next;
  }
  ha->head = quick_sort(ha->head, p, compar);
  return 0;
}
