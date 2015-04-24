
#Hash Tables

###Attached Files:
<ul>
<li>GenericHashTable.h - the Header file of the program</li>
<li>GenericHashTable.c - the Header impementation</li>
<li>Makefile - compiles and runs the app</li>
</ul>

###Added Functions:

<ul>int insertObject(Table* t,Object* obj,int loc) - find where to insert the object and instert it</ul>
<ul>void resizeHash(Table* t) - resize the Hash</ul>
<ul>void reinitHash(Table* t) - re-orders the  Hash after each resize</ul>
<ul>void printList(Table *table,int i) - prints the list in a specific cell</ul>
<ul>int countList(Object* obj) - count the list' size</ul>
<ul>void addToList(Object* list, Object* obj) - add to the list at the end</ul>

###IMPORTANT!

>The Makefile attached will help you run the app easier.

####How to:
1- Type 'make' to compile
2- Type 'make clean' to delete alll new files from compilation
3- Type 'make run' to run the application

>the Makefile will create a runable file called ex1.
