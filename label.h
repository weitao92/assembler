#ifndef LABEL_H
#define LABEL_H
//struct for label, add is the integer address, hex is the hex representation of address.
struct _label{
	int add;
	char name[33];
	char hex[9];
};

typedef struct _label label;

//a struct which contains 40 labels.
struct _labelList{
	label list[40];
	int size;
};

typedef struct _labelList labelList;

#endif