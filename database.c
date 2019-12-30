/*
 ============================================================================
 Name        : dbReader.c
 Author      : Stavros Mitsoglou
 Version     :
 Copyright   : Your copyright notice
 Description : A simple program to manage a small database of student
             : records using B-Trees for storage.
 ============================================================================
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

// To make the "C" implementation completely analogous to Java, one has to create
// an object for each student record and attach it to a corresponding bNode
// object in a B-Tree data structure.  These objects are represented by the
// corresponding structure templates below.

#define MAXLEN 20
#define false 0
#define true !false


// Structure Templates

typedef struct SR {				// The student record object
    char Last[MAXLEN];			// string for last name
	char First[MAXLEN];  		// string fir first name
	int ID;						// student id
	int marks;					// student grades
} SRecord;

typedef struct bN {				// The bNode object
	struct SR *Srec;			//pointer to SR
	struct bN *left;			//pointer to left branch
	struct bN *right;			//pointer to right branch
} bNode;

// Function Prototypes


bNode *addNode_Name(bNode *root, SRecord *Record);
bNode *addNode_ID(bNode *root, SRecord *Record);
bNode *makeNode(SRecord *data);


void inorder(bNode *root);
void search_Name(bNode *root, char *data);
void search_ID(bNode *root, int ID);
void str2upper(char *string);
void help();


// global variable to return the matching student record.


bNode *match;


// Main entry point is here.  Program uses the standard Command Line Interface

int main(int argc, char *argv[]) {

// Internal declarations

    FILE * NAMESIDS;        // File descriptor (an object)!
	FILE * MARKS;           // Will have two files open

    bNode *root_N;   		// Pointer to names B-Tree
    bNode *root_I;   		// Pointer to IDs B-Tree
    SRecord *Record;   		// Pointer to current record read in

	int NumRecords;
	char cmd[MAXLEN], sName[MAXLEN];
	int sID;

// Argument check
        if (argc != 3) {
                printf("Usage: sdb [Names+IDs] [marks] \n");
                return -1;
        }

// Attempt to open the user-specified file.  If no file with
// the supplied name is found, exit the program with an error
// message.

        if ((NAMESIDS=fopen(argv[1],"r"))==NULL) {
                printf("Can't read from file %s\n",argv[1]);
                return -2;
        }

        if ((MARKS=fopen(argv[2],"r"))==NULL) {
                printf("Can't read from file %s\n",argv[2]);
                fclose(NAMESIDS);
                return -2;
        }

// Initialize B-Trees by creating the root pointers;

    root_N=NULL;
	root_I=NULL;

//  Read through the NamesIDs and marks files, record by record.

	NumRecords=0;

	printf("Building database...\n");

	while(true) {

// 	Allocate an object to hold the current data

		Record = (SRecord *)malloc(sizeof(SRecord));
		if (Record == NULL) {
			printf("Failed to allocate object for data in main\n");
			return -1;
		}

//  Read in the data.  If the files are not the same length, the shortest one
//  terminates reading.

		int status = fscanf(NAMESIDS,"%s%s%d",Record->First,Record->Last,&Record->ID);
		if (status == EOF) break;
		status = fscanf(MARKS,"%d",&Record->marks);
		if (status == EOF) break;
		NumRecords++;

//	Add the record just read in to 2 B-Trees - one ordered
//  by name and the other ordered by student ID.

		root_N=addNode_Name(root_N,Record);
	    root_I=addNode_ID(root_I,Record);



	}

// Close files once we're done

	fclose(NAMESIDS);
	fclose(MARKS);

	printf("Finished, %d records found...\n",NumRecords);


//
//  Simple Command Interpreter: - This is commented out until you implement the functions listed above
//

	while (1) {
	    printf("sdb> ");
	    scanf("%s",cmd);					  // read command
	    str2upper(cmd);

	    if (0) {							  // This is a temporary stub
	    	continue;
	    }


// List by Name

		if (strncmp(cmd,"LN",2)==0) {         // List all records sorted by name
			printf("Student Record Database sorted by Last Name\n\n");
			inorder(root_N);
			printf("\n");
		}

// List by ID

		else if (strncmp(cmd,"LI",2)==0) {    // List all records sorted by ID
			printf("Student Record Database sorted by Student ID\n\n");
			inorder(root_I);
			printf("\n");
		}


// Find record that matches Name

		else if (strncmp(cmd,"FN",2)==0) {    // List record that matches name
			printf("Enter name to search: ");
			scanf("%s",sName);
			match=NULL;
			search_Name(root_N,sName);
			if (match==NULL)
			  printf("There is no student with that name.\n");
	        else {
			  if (strlen(match->Srec->First)+strlen(match->Srec->Last)>15) {
				printf("\nStudent Name:\t%s %s\n",match->Srec->First,match->Srec->Last);
			  } else {
				printf("\nStudent Name:\t\t%s %s\n",match->Srec->First,match->Srec->Last);
			  }
			  printf("Student ID:\t\t%d\n",match->Srec->ID);
			  printf("Total Grade:\t\t%d\n\n",match->Srec->marks);
	        }
		}

// Find record that matches ID


		else if (strncmp(cmd,"FI",2)==0) {    // List record that matches ID
			printf("Enter ID to search: ");
			scanf("%d",&sID);
			match=NULL;
			search_ID(root_I,sID);
			if (match==NULL)
			  printf("There is no student with that ID.\n");
	        else {
			  if (strlen(match->Srec->First)+strlen(match->Srec->Last)>15) {
				printf("\nStudent Name:\t%s %s\n",match->Srec->First,match->Srec->Last);
			  } else {
				printf("\nStudent Name:\t\t%s %s\n",match->Srec->First,match->Srec->Last);
			  }
			printf("Student ID:\t\t%d\n",match->Srec->ID);
			printf("Total Grade:\t\t%d\n\n",match->Srec->marks);
	      }
		}



// Help

		else if (strncmp(cmd,"H",1)==0) {  // Help
			help();
		}

		else if (strncmp(cmd,"?",2)==0) {     // Help
			help();
		}

// Quit

		else if (strncmp(cmd,"Q",1)==0) {  // Help
			printf("Program terminated...\n");
			return 0;
		}

// Command not understood

		else {
			printf("Command not understood.\n");
		}
	}

}

//
//	Write and insert the functions listed in the prototypes here.
//


//
//  Convert a string to upper case
//

void str2upper (char *string) {
    int i;
    for(i=0;i<strlen(string);i++)
       string[i]=toupper(string[i]);
     return;
}

//make node for binary tree
bNode *makeNode(SRecord *data)
{
	{
		bNode *node = (bNode*)malloc(sizeof(bNode));		// create new object
		node->Srec = data;									// initialize data (student record)
		node->left = NULL;									// set both successors
		node->right = NULL;									// to null
		return node;										// return handle to new object

	}
}

//add node to binary tree in terms of ID's
bNode *addNode_ID(bNode *root, SRecord *Record)
{
bNode *current;

// Empty tree

	if (root == NULL) {
		root = makeNode(Record);
	}

// If not empty, descend to the leaf node according to
// the ID number.

	else {
		current = root;
		while (true) {
			if (Record->ID < current->Srec->ID) {

// New ID < ID at node, branch left

				if (current->left == NULL) {				// leaf node
					current->left = makeNode(Record);		// attach new node here
					break;
				}
				else {									// otherwise
					current = current->left;				// keep traversing
				}
			}
			else {
// New ID >= ID at node, branch right

				if (current->right == NULL) {			// leaf node
					current->right = makeNode(Record);		// attach
					break;
				}
				else {									// otherwise
					current = current->right;			// keep traversing
				}
			}
		}
	}


	return root;
}

//add node in binary tree in terms of student name
bNode *addNode_Name(bNode *root, SRecord *Record)
{
	bNode *current;

	// Empty tree

		if (root == NULL) {
			root = makeNode(Record);
		}

	// If not empty, descend to the leaf node according to
	// the ID number.

		else {
			current = root;
			while (true) {
				if (strncmp(Record->Last,current->Srec->Last,MAXLEN)<0) {

	// New Last name < Last name at node, branch left

					if (current->left == NULL) {				// leaf node
						current->left = makeNode(Record);		// attach new node here
						break;
					}
					else {									// otherwise
						current = current->left;				// keep traversing
					}
				}
				else {
	// New Last name >= Last name at node, branch right

					if (current->right == NULL) {			// leaf node
						current->right = makeNode(Record);		// attach
						break;
					}
					else {									// otherwise
						current = current->right;			// keep traversing
					}
				}
			}
		}

		return root;

}

//inorder traversal for binary tree. Traverses the tree from smallest to biggest value depending
//on the value root points to (can be either id or last name)
void inorder(bNode *root)
{

		if(root->left !=NULL) inorder(root->left); //traverse left if not null

		//print student information at root
		printf("\nStudent Name:\t\t%s %s\n",root->Srec->First, root->Srec->Last);
		printf("Student ID:\t\t%d\n",root->Srec->ID);
		printf("Total Grade:\t\t%d\n",root->Srec->marks);

		if(root->right !=NULL) inorder(root->right); //traverse right if not null


}

//searches name using binary search
void search_Name(bNode *root, char *data)
{

			if(root!=NULL)
			{
			//if name at root matches name we are trying to search, set match to root
			if(strncasecmp(root->Srec->Last, data, MAXLEN) ==0)
			{
				match =root;
			}

			//if name at root is less (alphabetically) than name we are searching, traverse right
			 if(strncasecmp(root->Srec->Last, data, MAXLEN) < 0) search_Name(root->right,data);

			 //if name at root is greater (alphabetically) than name we are searching, traverse left
			 if(strncasecmp(root->Srec->Last, data, MAXLEN) > 0) search_Name(root->left,data);
			}


}

//searches student id using binary search
void search_ID(bNode *root, int ID)
{

			if(root!=NULL)
			{

			//if id at root is equal to id we are searching, set match to root
			if(root->Srec->ID == ID)
			{
				match =root;
			}

			//if id at root is numerically less than id we are searching, traverse right
			if(root->Srec->ID < ID) search_ID(root->right,ID);

			//if id at root is numerically greater than id we are searching, traverse left
			if(root->Srec->ID > ID) search_ID(root->left, ID);


			}



}



// Help
// prints command list

void help() {
	printf("LN List all the records in the database ordered by last name.\n");
	printf("LI List all the records in the database ordered by student ID.\n");
	printf("FN Prompts for a name and lists the record of the student with the corresponding name.\n");
	printf("FI Prompts for a name and lists the record of the student with the Corresponding ID.\n");
	printf("HELP Prints this list.\n");
	printf("? Prints this list.\n");
	printf("Q Exits the program.\n\n");

	return;
}




