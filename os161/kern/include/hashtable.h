#ifndef _HASHTABLE_H_
#define _HASHTABLE_H_

#include <types.h>
#include <kern/errno.h>
#include <kern/unistd.h>
#include <lib.h>

struct hashentry{
	char * key;
	void * entry;
};


struct hashtable {
	int size;
	char * name;
	struct hashentry ** table;
	struct hashentry deleted; // dummy
	struct hashentry reserved; // the place reserved so no one can occupy that
};




struct hashtable * ht_create(int size, char * name);
int ht_add_element(struct hashtable * ht, char * key, void * ptr);
int ht_delete_element_by_index(struct hashtable * ht, int index, int destroy);
int ht_delete_element_by_key(struct hashtable * ht, char * key, int destroy);
void * ht_retrieve_element_by_index(struct hashtable * ht, int index);
void * ht_retrieve_element_by_key(struct hashtable * ht, char * key);

unsigned int hashfunction (char* key);



#endif /* _HASHTABLE_H_ */

