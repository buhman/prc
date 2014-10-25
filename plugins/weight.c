#include "prc.h"
#include "dll.h"
#include "weight.h"

unsigned long int
weight_add(dll_t *weights, 
	   unsigned long int weight,
	   const char *fact)
{
  weight_t *item;
 
  item = calloc(1, sizeof(weight_t));
 
  item->weight = weight;
  item->fact = strdup(fact);

  dll_enq(weights, item);

  item = weights->head->buf;
  item->sum += weight;

  return item->sum;
}

char*
weight_select(dll_t *weights)
{
  long int num;
  dll_link_t *link;
  weight_t *item;

  item = weights->head->buf;
  num = prc_rand_lim(item->sum);

  for (link = weights->head;;
       link = link->next) {
    
    item = link->buf;
    num -= item->weight;

    if (num < 0)
      return item->fact;
    if (link->next == NULL)
      return NULL;
  }
}
