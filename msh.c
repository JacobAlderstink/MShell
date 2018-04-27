/*
Jacob Alderink
Axel Kring CS-240-01
Septermber 25 2017
last edited: October 24 2017
msh.c a3

*/


//********Libraries*****************
#include <stdio.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

//**********Prototypes***************
void start(); //opens .mshrc and reads its files
void run();   //begins the loop of parsing strings from the standard 
void analyze(char** strs);
char *Getinput();
void ProcessArgs(char* str);
char **getArgs(char* str);
void ShellCommand(char** command);
void AddToHistory(char* string);
void SpecificHistory(char* str);


//******Global Variables*************
char *History[20];
int counter = 0;
int num_commands = 0;
char *Alias[40][2];
//*******msh begin*******************


int main(){
    start();

    run();

  return 0;
}


void start(){
  for(int i = 0; i < 20; i++){
    History[i] = NULL;
  }
  //****************Intizilizes these values to 0***********
  for(int i = 0; i < 40; i++){
    Alias[i][0] = NULL;
    Alias[i][1] = NULL;
  }
  
  FILE* startup;
  startup = fopen(".mshrc","r");
  if(startup == NULL)
    {
      printf("Error Could Not Open .mshrc make sure the file is located in the current active directory\n");
      return;
    }
  size_t t = 0;
  char *lineptr;
  
  if(getline(&lineptr, &t, startup) == -1){
    return;
  }
  else{

    do{
      
      ProcessArgs(lineptr); //grabs one line at a time till end of file
      
    }
    while(getline(&lineptr, &t, startup)!=-1);

    free(lineptr);

    fclose(startup);

  }
}



void run(){
  
  char *string;
  char *exitstr = strdup("exit"); //exit status string	string = NULL;

  do{
    printf("?:");
    string = Getinput();
    ProcessArgs(string);
    
  }while(strcmp(string,exitstr) != 0);
    
}



char *Getinput(){
  char *str;
  size_t size = 10;
  int ch;
  size_t len = 0;
  
  str = (char*)malloc(sizeof(char)*size);

 while(EOF != (ch=fgetc(stdin)) && ch != '\n'){ //grabs the string until it ends

    str[len++]=ch;

    if(len==size){
      str=(char*)realloc(str, sizeof(char)*(size+=16));

      if(!str)
	return str;

    }
  }
  str[len++]='\0'; //ends the string with a NULL character

  AddToHistory(str);
  return (char*)realloc(str, sizeof(char)*len); //returns the string

}




void ProcessArgs(char *str){
  char *token;
  char *tokenarray[10]; //limit of 10 commands on the same line with ; as the seperator
  int i;
  char **args;
  
  for(i = 0; ; i++, str = NULL){
    token = strtok(str,";"); //places token as the command to be processed
    if(token == NULL)
      break;
    //printf("%s\n",token);
    tokenarray[i] = token; //places in double array
  }
  
  tokenarray[i] = NULL;

  for(int j = 0; j < i; j++){ 
    args = getArgs(tokenarray[j]); //  
    analyze(args);                 //  processes in order the commands to be executed
    free(str);
    free(args);
  }
  
}





char **getArgs(char *str){

  size_t size = 20;
  char *temp;
  
 char **argument = (char**)malloc(sizeof(char*)*size); //holds the string array that will hold each argument of the command


  temp = strtok(str, " \t\n\a\r"); //tells strtok to split the string up when it runs into a tab, newline, space, etc.
  int i = 0;
 while(temp != NULL){
    argument[i] = temp;
    i++;

    if(i>=size){
      size += 20;
      argument = (char**)realloc(argument, size*sizeof(char*));
    }
    temp = strtok(NULL," \r\t\n\r");

  }

 argument[i] = NULL; //NULLs the next argument
 num_commands=i-1;
 return (char**)realloc(argument, sizeof(char*)*size); //returns the string
}




void analyze(char** strs){
  for(int i = 0; i < 40; i++){
    if(Alias[i][0] != NULL){ //first tests to see if its NULL
      if(strcmp(strs[0], Alias[i][0]) == 0) //then tests to see if its the right alias
	 {
	   //strs[0]=Alias[i][2];
	   ProcessArgs(Alias[i][1]); //if so process the aliased argument instead 
	   return;
	 }
    }
  }
  
  char* Shellcommands[] = {"cd","history", "alias", "export", "!!","unalias"};
  for (int i = 0; i < 6; i++) {
    if (strcmp(strs[0], Shellcommands[i]) == 0)
      {
	ShellCommand(strs); //tests to see if command entered was a shell command. If so enters this function
	return;
      }
  }
   if(strs[0][0] == '!')
    SpecificHistory(strs[0]);
  
  pid_t childpid;
  int status;
  
  if((childpid = fork()) == 0){
    execvp(strs[0], strs); //Child Process, forks and searches in Path for the command to be executed then uses the argument string array for specifics.
    exit(0); //exits when finished;
  }
  else{
    wait(&status); //Parent Process, will wait until Child is finished and once the child is finished it will begin the shell again.
  }
  
  
}



void ShellCommand(char** command){
  char* Shellcommands[] = {"cd","history", "alias","export", "!!", "unalias"};

  
  //*************For cd command*******************
  if(strcmp(command[0],Shellcommands[0]) == 0){
    if (command[1] == NULL) {
      printf("msh expected argument for cd\n"); //test to see if they entered in  a destination file
    }
    else {
      chdir(command[1]); //simply changes directory to specified directory
    } 
  }
  
  
  //*************For history Command**************
  if(strcmp(command[0], Shellcommands[1]) == 0){
    
    for( int j = 0; j < 20; j++){
      if(History[j] == NULL)  
	break;
      printf("%d: %s\n", j, History[j]);   //prints out larger number first so the std look like this :         1  ls
    }                                                   //                                                      2  ps
                                                        //                                                      3  which ps ; hi ; no;     
  }                                                     //                                                      4  history
  

  //*************   !!   ***************************
  if(strcmp(command[0], Shellcommands[4]) == 0){

    
    ProcessArgs(History[counter - 2]); //minus 2 because it needs to be the previous one and !! has alread been logged into history as well
    
  }
  
  
  //************For alias Command*****************
  if(strcmp(command[0], Shellcommands[2]) == 0){
    char *temp = (char*)malloc(sizeof(command[1])*num_commands); 
    temp = strchr(command[1],'='); //sets set to the new addition to alias;
    size_t size = 40;
    //printf("%s\n%s\n",temp, command[1]); //=ls,ls=ls
    
    size_t t = (strlen(temp));
    size_t t2 = (strlen(command[1]));
    
    int i = 0;
    while(Alias[i][0] != NULL){
      i++;
    }
    int j = 2;
    
    char* temp2;
    while(command[j] != NULL){ 
      temp2 = strdup(command[j]); //copys the command to temp2
      strcat(temp,  " "); //adds a space between them
      //printf("%s\n", temp);
      strcat(temp, temp2); //adds the next command onto the alias
      //printf("%s\n", temp);
      j++;
    }
    //printf("temp= %s\n", temp);
    
    char* aliased = strndup(command[1], t2-t);
    Alias[i][0] = aliased;
    temp+=2; //gets rid of '=' and '"' inside the alias command exp. alias ls="ps"
    t=strlen(temp); //
    Alias[i][1] = strndup(temp,t-1);//gets rid of last '"' acoording to alias syntax
    //printf("alias = %s\ncommand = %s\n",Alias[i][0],Alias[i][1]);
  }
  


  //************For Unalias Command***************
  if(strcmp(command[0], Shellcommands[5]) == 0){
    int i = 0;
    while(1){
      if(i>40){
	printf("error no alias under the name \"%s\"\n",command[1]); //if all slots in Alias have been tested, then error
	return;
      }
      if(Alias[i][0]==NULL){ //tests to see if it equal to NULL first, if it is then continue on in while
	i++;
	continue;
      }
      if(strcmp(Alias[i][0],command[1])==0){ //if not NULL then test to see if it is equal to the alias
	
	
	
	//printf("%s\n%s\n",Alias[i][0],Alias[i][1]);
	
	Alias[i][0] = NULL; //once found set alias and aliased equal to NULL
	Alias[i][1] = NULL;
	break;
	
      }
    }
  }
  
  
  //************For export Command*****************
  if(strcmp(command[0], Shellcommands[3]) == 0){
    char* newPath; 
    char* env;
    char* set;
    env = strndup(command[1],4); //sets env = PATH
    newPath = getenv(env);       // sets newPath = current Path

    //printf("%s\n",env);
    set = strchr(command[1],':'); //sets set to the new addition to path;
    //printf("%s\n",set);

    char* value = strcat(newPath,set); //concatenates the current path and the addition together
    setenv(env,value,1); // sets the Path;
    
  }
  }



void AddToHistory(char* string){
  counter++;
  for(int i = 0; i < 20; i++){
    if(History[i] == NULL)
      {
	History[i] = string; //increases counter and adds current string into history
	return;
      }
  }
}



void SpecificHistory(char* str){
  //int i = 1;
  // int j = 0;
  char *partofhistory;


  partofhistory = strchr(str,'!'); //grabs everything after the ! mark
  partofhistory++; // increments by 1

  int temp = atoi(partofhistory); //changes the string to a int
  ProcessArgs(History[temp]); //and then process the argument at whatever number they entered.
}
  
  
