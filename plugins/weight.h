typedef struct weight weight_t;

struct weight {
  long int weight;
  long int sum;
  char *fact;
};

unsigned long int
weight_add(dll_t *weights, 
	   unsigned long int weight,
	   const char *fact);

char*
weight_select(dll_t *weights);
