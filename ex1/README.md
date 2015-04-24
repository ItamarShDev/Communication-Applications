
#Hash Tables

###Attached Files:
<li>GenericHashTable.h - the Header file of the program</li>
<li>GenericHashTable.c - the Header impementation</li>
<li>Makefile - compiles and runs the app<li>


###Added Functions:
<li>int insertObject(Table* t,Object* obj,int loc) - find where to insert the object and instert it</li>
<li>void resizeHash(Table* t) - resize the Hash</li>
<li>void reinitHash(Table* t) - re-orders the  Hash after each resize</li>
<li>void printList(Table *table,int i) - prints the list in a specific cell</li>
<li>int countList(Object* obj) - count the list' size</li>
<li>void addToList(Object* list, Object* obj) - add to the list at the end</li>

###IMPORTANT!

>The Makefile attached will help you run the app easier.

####How to:
1- Type 'make' to compile
2- Type 'make clean' to delete alll new files from compilation
3- Type 'make run' to run the application

>the Makefile will create a runable file called ex1.
