#define INT_TYPE 0
#define STR_TYPE 1

typedef struct Object
{
	void* data;//the data
	//list pointers
	struct Object *next;
	struct Object *prev;
	int index;//place index
	int weight, port, _rec;
	char *pi, *ip;
} Object;

typedef struct Table
{
	Object** arr;
	int listLength; //to dave the list' maxed length
	int type;//data type
	int d;//for the hash func
	int OriginSize;
	int curr_size;
} Table;

/**
* The function gets the original size and the type of the data in the table elements.
* it initializes the Table struct members.
* On success, the function returns a pointer to the new created Table, otherwise, it return NULL.
*/
Table* createTable(int size, int dType, int listLength);
/**
* The function release all the allocated members of struct Table.
*/
void freeTable(Table* table);

/**
* The function adds data to the hashtable (as described in the exe definition)
* On success, the function returns the array index of the added data, otherwise, it return -1.
*/
int add(Table* table,const void *data,const void* b_data, int weight);

/**
* The function removes the Object which its data equals to data, if there are more than one, it removes the first one.
* On success, the function returns the array index of the removed data, otherwise, it returns -1.
* -1 is also returned in the case where there is no such object.
*/
int removeObj(Table* table,const void* data);

/**
* The function search for an object that its data is equal to data and returns a pointer to that object.
* If there is no such object or in a case of an error, NULL is returned.
*/
Object* search(Table* table,const void* data);

/**
* The function print the table (the format is in the exe definition)
*/
void printTable(Table* table);

/**
* This function creates an object and return the pointer to it or NULL if there is some error.
*/
Object* createObject(int type,const void* data);

/**
* This function frees an object,  note the in the case of STR_TYPE, the data
* of the object should also be freed.
*/
void freeObject(Object* obj, int type);

/**
* check the equality of the data of two objects. The implementation is different depending the type of the data.
* the function returns 0 if they are equal or some other value if they are not equal.
*/
int isEqual(int type,const void* data1,const void* data2);

/**
* returns the hash value of an integer, which is key mod origSize
*/
int intHashFun(int* key, int origSize);

/**
* returns the hash value of an string, which is m mod origSize, where m is the sum of the ascii value of all the
* character in key.
*/
int strHashFun(char* key, int origSize);

//Added Functions
/*
*this function insert the object to its place according to the factors
*/
int insertObject(Table* t,Object* obj,Object *b_obj,int loc);
//this function doubles the array size
//then call reinitHash to re-order it
void resizeHash(Table* t);
/*reinsert the objects
*put any existing list in cell i at cell 2i
*this function runs backwards to prevent double cell moving
*/
void reinitHash(Table* t);
//print every list in a pecific cell
void printList(Table *table,int i);
//this function count the number of objects in a given list
int countList(Object* obj);
/*
* this function adds an object to the end of the list
*
*/
void addToList(Object* list, Object* obj);
//add the neighbore
//void addVers(void* a, void* b);
// to ?get the weight of traveling between a and b
int getWeight(Table* t,const void* a,const void* b);
int addObject(Table *table,const void *data,int port,  char* ip);