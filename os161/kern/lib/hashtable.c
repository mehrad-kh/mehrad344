#include <hashtable.h>
#include <types.h>
#include <kern/errno.h>
#include <kern/unistd.h>
#include <lib.h>
#include <lib.h>

struct hashtable * ht_create(int size, char * name)
{
	/* allocate an hashtable structure */
	struct hashtable * ht = (struct hashtable *)kmalloc(sizeof(struct hashtable));
	if (ht == NULL)
		return NULL;

	/* set the hashtable name */
	ht->name = (char *)kmalloc((strlen(name)+1)*sizeof(char));
	if (ht->name == NULL)
		return NULL;
	strcpy(ht->name, name);

	ht->size = size;

	/* Allocate an array of struct hashentry * of the size of "size" */
	ht->table = (struct hashentry **)kmalloc(size*sizeof(struct hashentry *));
	if (ht->table == NULL)
		return NULL;

	/* Initially set all void * pointer to NULL*/
	int i;
	for (i = 0; i < size; i++)
		ht->table[i] = NULL;

	return ht;

}


int ht_add_element(struct hashtable * ht, char * key, void * ptr)
{
	assert(ht != NULL);
	assert(key != NULL);
	assert (ptr != NULL);

	int size = ht->size;

	/* get the hashkey corresponding */
	unsigned int index = hashfunction(key);
	//int index =1;
	index = (index % size);

	int offset = 0;

	/* right now implementing linear probing */
	while (ht->table[(index + offset)% size] != NULL &&  
		ht->table[(index + offset)% size] != &(ht->deleted) && offset != size)
		offset++; 

	/* when get out of while either a slot is found which is (index+offset)%size 
	or offset == size which means no slot is found */

	if (offset == size)
		return -1; 	// indicates no slot is available 

	index = (index+offset)%size;
	
	ht->table[index] = (struct hashentry *)kmalloc(sizeof(struct hashentry));

	if (ht->table[index] == NULL)
		return -1; 	// indicates no memory available

	ht->table[index]-> key = (char *)kmalloc((strlen(key)+1)*sizeof(char));
	if(ht->table[index]->key == NULL) {
		return -1;
	}
	strcpy(ht->table[index]-> key, key);

	ht->table[index]-> entry = ptr;


	return index;	    
}


int ht_delete_element_by_index(struct hashtable * ht, int index, int destroy)
{
	int size = ht->size;
	assert (ht != NULL);
	assert(index < size && index > 0);

	/* make sure item exist at position index */
	if (ht->table[index] == NULL || ht->table[index] == &(ht->deleted))
		return -1;   // indicating no item exist to be deleted

	/* Otherwise the item exist. if destroy flag is set
	   then the entry item gets freed in addition to being detached from 
	   the the table. Else just gets detached but not freed */

	if (destroy)
		kfree(ht->table[index]->entry);
		
	kfree(ht->table[index]->key);
	kfree(ht->table[index]);
	
	ht->table[index] = &(ht->deleted);

	return 0;
}


int ht_delete_element_by_key(struct hashtable * ht, char * key, int destroy)
{
	assert(ht != NULL);
	assert(key != NULL);
	
	int index = hashfunction(key);
	int size = ht->size;
	int offset = 0;

	while (ht->table[(index + offset) % size] != NULL && 
		strcmp(ht->table[(index + offset) % size]->key, key) != 0 && offset != size)
		offset++;

	if (offset == size || ht->table[(index + offset) % size] == NULL)
	{
		// item with given key not exist
		return -1;
	}

	// Otherwise key is found
	index = index + offset;

	if (destroy)
		kfree(ht->table[index]->entry);
		
	kfree(ht->table[index]->key);
	kfree(ht->table[index]);
	
	ht->table[index] = &(ht->deleted);

	return 0;


}


void * ht_retrieve_element_by_index(struct hashtable * ht, int index)
{
	
	assert(ht != NULL);

	/* make sure item exist at position index */
	if (ht->table[index] == NULL || ht->table[index] == &(ht->deleted))
		return NULL;   // indicating no item exist to be retrieved

	return ht->table[index]->entry;
}

void * ht_retrieve_element_by_key(struct hashtable * ht, char * key)
{
	assert (ht != NULL);
	assert(key != NULL);

	int index = hashfunction(key);
	int size = ht->size;
	int offset = 0;

	while (ht->table[(index + offset) % size] != NULL && 
		strcmp(ht->table[(index + offset) % size]->key, key) != 0 && offset != size)
		offset++;

	if (offset == size || ht->table[(index + offset) % size] == NULL)
	{
		// item with given key not exist
		return NULL;
	}

	// Otherwise key is found
	index = index + offset;

	return ht->table[index]->entry;
}


unsigned int hashfunction (char* key) 
{
	unsigned int hash = 101;
	int i = 0;
	for (i = 0; i< strlen(key); i++) {
		hash = hash*101+key[i];
	}
	return hash;
}


