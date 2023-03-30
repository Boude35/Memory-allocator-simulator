
 

#include <iostream>
#include <fstream>
#include <vector>
#include<algorithm>
#include "Token.hpp"

using namespace std;

//This is structure will hold each one of the "block" that will be allocated in memory
struct variablenode {
	string name = ""; //name of the "variable"
	int addres = 0; //starting addres in memory
	int size = 0;// size of the block
	int reference = 0; //how many "variables" are pointing to this block
	bool free = true;//is this space empty or not?
};

/**********
GRAMMAR for the Parser:

<prog> --> <slist>
<slist> --> <stmt> SEMICOLON <slist> | (Nothing)
<stmt> --> ID LPAREN ID RPAREN | 
	ID LPAREN RPAREN | 
	ID ASSIGNOP <rhs>
<rhs> --> ID LPAREN NUM_INT RPAREN | ID

 **********/

//Forward Declaration of all the function used for the parser
void prog(ifstream&, vector<variablenode>&);
void slist(ifstream&, vector<variablenode>&);
void stmt(ifstream&, vector<variablenode>&);
void rhs(ifstream&, vector<variablenode>&);

//Check if the program runs succesfully, false if not
bool isValid = true;	
//string that will store the parsed code
string result;
//token that will be readed each time
Token tok;


int main(int argc, char *argv[])
{
  //list of all the differents blocks
  vector<variablenode> variable;
  //variable that will hold the inputed size by the user
  int list;
  cout << "Please enter the initial freelist (heap) size: ";
  cin >> list;
  
  //push the first block that will represent the initial empty space
  variable.push_back(variablenode());
  //since it is empty name is null
  variable[0].name = "null";
  //no "variable" is referncing this value
  variable[0].reference = 0;
  //empty so free is true
  variable[0].free = true;
  //its size is going to be the one inputed by the user
  variable[0].size = list;
  //starting at "index" 0
  variable[0].addres = 0;
  
  /**********
	Variable needed to open and read from the file that stores 
	the instructions neede fro the execution of the program

 **********/
  string file;
  cout << "Please enter name of an input file: ";
  cin >> file;
  ifstream ifile(file.c_str());

  // if open was not successful, let user know. 
  if (!ifile)
    {
      cerr << "ERROR: Could not open file: \"" << file << "\"" <<endl;
      return -1;
    }	
  
  //call the "parent" function of the parser
  prog(ifile, variable); 

  return 0;
}

//variable that will holds the name of the "variables" so they can be called from different functions
string nombre;

/**********
<prog> --> <slist>
 **********/
void prog(ifstream &ifile, vector<variablenode>& variable)
{
	tok.get(ifile);//get the next token
	slist(ifile, variable);
	
}

/**********
<slist> --> <stmt> SEMICOLON <slist> | (Nothing)
 **********/

void slist(ifstream &ifile, vector<variablenode>& variable)
{
	if(tok.type() == ID)
	{
		stmt(ifile, variable); //call stmt
		if(tok.type() == SEMICOLON)//next token must be a semicolon
		{
			tok.get(ifile);//read next token
			slist(ifile, variable);
		}
		else//else, we have an error
			isValid = false;
	}
	

}


//function that eliminates a "variable" from the list when few variables point the same block
void free2(variablenode newVa, vector<variablenode>& variable)
{
	 int count;//will hold the index of the variable that will be erased
	 int i = 0;//keep updating until we find the value we want to eliminate
	 vector<variablenode>::iterator ptr = variable.begin();//iterator to  get through the list
	 
	 for( ; ptr != variable.end(); ptr++)
	 {
	 	if(newVa.name == ptr->name)//if the variable is the one desired to eliminate
	 		count = i+1;//store the index
	 }
	 variable.erase(variable.begin() + count);//delete it from the vector
}

//funtion to free the space occupied by a variable
void free(string name, vector<variablenode>& variable)
{
	for (auto & element : variable)//iterate through the variable 
	{
		if(name == element.name)//if the element in the vector is the one we eant to delete
		{
			if(element.reference == 1)//and it is only being referenced once
			{
				element.reference = 0;//decrement the references
				element.free = true;//"give it" back to the free list
				element.name = "null";//change the name to null since it is empty
			}
			else //if the value is referenced more than once
			{
				for (auto & element2 : variable) 
				{
					if(element.addres == element2.addres)
					{
						element2.reference -= 1;//decrement the value of all the variables that referenced the same value
					}
				}
				free2(element, variable);//eliminate the value from the vector but dont give this space back to the freelist
			}
		} 	
	}	
}

//function to display the variales and the free list current state
void dump(vector<variablenode>& variable)
{
	cout << "Variables: "<<endl; //display variables
	for (auto & element : variable) 
	{
		if(element.free == false) //being a variable means that you are not empty
		{
			//display the name, address, size and references
			cout << element.name;
			cout << ": ";
			cout << element.addres;
			cout << "("; 
			cout << element.size; 
			cout << ")";
			cout << " [";
			cout << element.reference;
			cout << "]";
			cout << endl;
		}
	}
	cout << "Free List: "<<endl; //display free list
	bool comma = false;
	for (auto & element : variable) 
	{
		if(element.free == true)//being an empty block means you are empty
		{
			if(comma == true)//statement to not show the final comma
			{
				cout << ", ";
			}
			//display the address, size and references since the name is null(not important)
			cout << element.addres;
			cout << "("; 
			cout << element.size; 
			cout << ")";
			cout << " [";
			cout << element.reference;
			cout << "]";
			comma = true;
		}
	}
	cout << endl;
	cout << "===============================================================" << endl;
}

//function that joins together two adjacent empty blocks
void compress(vector<variablenode>& variable)
{
	 //iterator to iterate trhough the list "in pairs"
	 vector<variablenode>::iterator ptr = variable.begin(); //point to the first value
	 vector<variablenode>::iterator ptr2 = variable.begin() + 1;//points to the next value with respect to ptr
	 
	 for( ; ptr2 != variable.end();)//iterate the list
	 {
	    if(ptr->free == true && ptr2->free == true)//if both iterators(adjacents blocks) point empty blocks
	    {
	    	//merge them by adding the size of the second block to the first one and deleting the second one from the list
	    	
	    	//display this merging
	    	cout << "Merging with ";
	    	cout << ptr->addres;
		cout << "("; 
		cout << ptr->size; 
		cout << ")";
		cout << " [";
		cout << ptr->reference;
		cout << "]";
		
		ptr->size += ptr2->size;
		
		cout << " <--> ";
		
		cout << ptr2->addres;
		cout << "("; 
		cout << ptr2->size; 
		cout << ")";
		cout << " [";
		cout << ptr2->reference;
		cout << "]";
		cout << endl;
		
		//delete the second empty block
	    	ptr2 = variable.erase(ptr2);
	    }
	    else
	    {
	    	//if both block are not empty move to the next ones, not needed in the previous when both are empty since when we delete one element iterator jump to the next value
		ptr++;
		ptr2++;
	    }
	}
	 
}

/**********
<stmt> --> ID LPAREN ID RPAREN | 
	ID LPAREN RPAREN | 
	ID ASSIGNOP <rhs>

 **********/
void stmt(ifstream &ifile, vector<variablenode>& variable){
	if(tok.type() == ID)
	{
		nombre = tok.value();//store the value because is the name of the variable or the keyword that tells us to dump or compress
		tok.get(ifile);
		if(tok.type() == LPAREN)
		{
			tok.get(ifile);
			if(tok.type() == ID)
			{
				nombre = tok.value();
				tok.get(ifile);
				if(tok.type() == RPAREN)
				{
					free(nombre, variable);//if we reach this point means that we want to free some spacr
					tok.get(ifile);
				}
				else//else, we have an error
					isValid = false;
			}
			else if(tok.type() == RPAREN)
			{
				//if we reach this point it means that we want either to compress or dump
				
				//depending of the value we reade previously we dump or compress
				if(nombre == "dump")
				{
					dump(variable); //call dump
				}
				else if(nombre == "compress")
				{
					compress(variable);//call compress
				}
				tok.get(ifile);
			}
			else//else, we have an error
				isValid = false;
		}
		else if(tok.type() == ASSIGNOP)
		{
			tok.get(ifile);
			rhs(ifile, variable);
		}
		else//else, we have an error
			isValid = false;
	}
	else//else, we have an error
		isValid = false;
}

//function to implement first fit as the memory allocator system. It will iterate through the list to find the first empty space that is at least as big as the variable wanted to insert
void firstfit(variablenode newVa, vector<variablenode>& variable)
{
	bool alreadyIn = false; //variable that controls if we already inserted the value
	for (auto & element : variable) 
	{
    		if(element.free == true && newVa.size <= element.size && alreadyIn == false) //if the value is not in there yet, the block is empty and the space in big enough
    		{
    			//give the values to the variable
    			newVa.free = false;
    			newVa.reference = 1;
    			newVa.addres = element.addres;
    			element.size -= newVa.size;
    			element.addres += newVa.size;
    			//add it to the list
    			variable.push_back(newVa);
    			//It is in the list!
    			alreadyIn = true;
    		}
	}
	//using the sort library, sort the block in ascending addres values.
	sort(variable.begin(), variable.end(), [](const variablenode& lhs, const variablenode& rhs) {
      return lhs.addres < rhs.addres;
   });
	
}

//function that handles the references of a block
void reference(variablenode newVa, variablenode newVa2, vector<variablenode>& variable)
{
	for (auto & element : variable) 
	{
    		if(element.name == newVa2.name) //if we find the variable being referenced in the list
    		{
    			element.reference += 1; //add one to its reference count
    			newVa2 = element; //store it for future uses
    			string nombre = newVa.name; //store the name of the referrer
    			newVa = newVa2; //make the referrer eqaul the variable being referenced
    			newVa.name = nombre; //but give it the ame that correspond
    			variable.push_back(newVa);//add this referrer to the list
    			
    		}
	}
	for (auto & element : variable) 
	{
		if(element.addres == newVa2.addres)//find the already existing values that already referenced that value (which means that they have the same address)
    		{
    			//make them equal but give them their appropiate name (basically update the reference count because they were already equal)
    			string nombre = element.name;
    			element = newVa2;
    			element.name = nombre;
    		}
	}
	
}


void rhs(ifstream &ifile, vector<variablenode>& variable)
{
	bool allocation = false;
	if(tok.type() == ID)//if first token is an ID
	{
		variablenode newVa;
		newVa.name = nombre;
		string nombre2 = tok.value();
		tok.get(ifile);//get the next token
		if(tok.type() == LPAREN)//if it is a parethesis
		{
			//if the variable is new it wont free anything, but if it is a redeclaration it will empty the space occupied by that variable to redeclare it with new values
			free(nombre, variable);
			allocation = true;
			tok.get(ifile);//read next token
			if(tok.type() == NUM_INT)//if it is a num integer
			{
				newVa.size = stoi(tok.value());
				tok.get(ifile);//read next token
				if(tok.type() == RPAREN)//if it is a parenthesis
				{
					tok.get(ifile);//read next token
					firstfit(newVa, variable);//insert the value in the list
				}
				else//if it is not an ID, we have an error
					isValid = false;
			}
			else//if it is not an ID, we have an error
				isValid = false;
			
		}//else 
		
		//The if statement will be executed only when one variable equals another and not when we are allocation spacing for a variable
		if(allocation == false)
		{
			//variable taht will hold the info of the variable being referenced
			variablenode newVa2;
			//pass the name of the variable being referenced that was readed by the parser
			newVa2.name = nombre2;
			//if we are not making the variable equal itself
			if(nombre2 != nombre)
				reference(newVa, newVa2, variable); //call reference
		}
	}
	else//if it is not an ID, we have an error
		isValid = false;	
}





