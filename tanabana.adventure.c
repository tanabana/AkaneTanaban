/* ********************************************************
 * Author: Akane Tanaban
 * Date: May 3, 2015
 * Title: Program 2 - Adventure
 * Description:
 * This program first creates a series of files that hold descriptions of
 * the rooms and how they are connected, and then offer to the player
 * an interface for playing the game using those generated rooms.
 * ********************************************************/
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h> //used for integer types ssize_t
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>


struct room {
	char* name;
	char* type;
	int connect_num;	//number of connections for a given room
};

/* The makeRooms function generates the room information and stores into files.
 * It also generates an adjacency matrix that stores information about
 * about which rooms are connected to each other.
 */
void makeRooms(char* directory, struct room* a_room, int a_room_size, int connections[][7], int connect_size);

/* The playGame function is where the user interacts with the user interface
 * to play the game 
 */
void playGame(char* directory);

int main(int argc, char* argv[])
{
	/* Initialize random number generator */
	time_t t;
	srand((unsigned) time(&t));

	/* Create output directory <username>.rooms.pid */
	char* username = argv[0];
	char process_id_str[10];
	int process_id = getpid();			//get process id number
	
	sprintf(process_id_str, "%d", process_id);	//convert pid to string

	char* directory = strtok(username, ".");	//just get username portion from name of executable
	strcat(directory, ".rooms.");
	strcat(directory, process_id_str);	
	
	int check = mkdir(directory, S_IRWXU);

	/* Generate room files with the makeRooms function.*/
	struct room a_room[7];
	int a_room_size = 7;
	int connections[7][7];
	int connect_size = 7;
	makeRooms(directory, a_room, a_room_size, connections, connect_size);

	/* Play game */
	playGame(directory);

	return 0;
}

void makeRooms(char* directory, struct room* a_room, int a_room_size, int connections[][7], int connect_size)
{
	/* Make connections adjacency matrix and initialize */
	/* Diagonal values set to -1 means there cannot be a connection to itself.*/
	/* Value of 0 means there is no connection, and 1 means there is a connection.*/
	int i = 0;
	int j = 0;

	for (i = 0; i < 7; i++)
	{
		for (j = 0; j < 7; j++)
		{
			if (i == j)
			{
				connections[i][j] = -1;
			}
			else
			{
				connections[i][j] = 0;
			}
		}
	}

	char* room_name[10] = {
		"twisty",
		"PLOVER",
		"Dungeon",
		"cellar",
		"attic",
		"salon",
		"hall",
		"kitchen",
		"diningroom",
		"library"
	};

	char* room_type[3] = {"START_ROOM", "END_ROOM", "MID_ROOM"};

	FILE *output_file_ptr;
	int path_length;	//directory path length
	char* temp;
	char k_str[2];
	char* file_path = directory;

	strcat(file_path, "/room");
	
	int num_of_connections;
	int room_name_idx = 0;
	int connection_num;
	int chosen_name_idx[7];
	
	/* Choose 7 room names and room types */
	i = 0;
	for (i = 0; i < 7; i++)
	{
		chosen_name_idx[i] = -1;		//initialize indices to -1
	}
	i = 0;
	int q = 0;
	while(i < 7)
	{
		room_name_idx = rand() % 10;		//generate number from 0 to 9
		int m;
		for(m = 0; m < 7; m++)	//if name has been chosen, choose another index number
		{
			if(chosen_name_idx[m] == room_name_idx)
			{
				room_name_idx = rand() % 10;
				m = -1;
			}
		}
		chosen_name_idx[q] = room_name_idx;
		q++;
		a_room[i].name = room_name[room_name_idx];
		 
		if(i == 0)
		{
			a_room[i].type = room_type[0];

		}
		else if(i == 6)
		{
			a_room[i].type = room_type[1];
		}
		else
		{
			a_room[i].type = room_type[2];
		}
		i++;
	} //end of choosing rooms
	
	/* Assign rooms to connect to for each room */

	j = 0;
	int k = 0;
	for (k = 0; k < 7; k++)	//loop through all 7 rooms to make connections and output rooms to files
	{
		sprintf(k_str, "%d", k);
		strcat(file_path, k_str);

		int remaining_num_connections = 0;

		a_room[k].connect_num = rand() % 4 + 3;	//make 3-6 connections per room

		//check how many connections this room already has
		int num = 0;
		for (j = 0; j < 7; j++)
		{
			if (connections[k][j] == 1)
			{
				num++;
			}
		}

		//if number of current connections exceeds the random num connections,
		//select number of current connections as its assigned num of connections.
		if (a_room[k].connect_num < num)
		{
			a_room[k].connect_num = num;
			remaining_num_connections = 0;
		}
		else
		{
			//check that there are sufficient open connections elsewhere
			//to accomodate randomly chosen num of connections.
			//if there are not enough openings, choose current connection num to be its
			//assigned num of connections.
			int count = 0;
			int s = 0;
			int t = 0;
			for (s = 0; s < 7; s++)
			{
				for (t = 0; t < 7; t++)
				{
					if (connections[s][t] == 0)
					{
						count++;
					}
				}
			}

			count = count - (6 - num);	//total number of open connections minus open connections to itself
			if (a_room[k].connect_num > count)
			{
				a_room[k].connect_num = num;	//if not enough openings, choose current num of connections as num of connections
				remaining_num_connections = 0;
			}
			else
			{
				remaining_num_connections = a_room[k].connect_num - num;
			}
		}
		

		for (j = 0; j < remaining_num_connections; j++)	//loop through number of remaining connections for each room
		{
			connection_num = rand() % 7;

			// check that the connection does not already exist, or the connection is with itself
			// if yes, then choose another random connection
			while (connections[k][connection_num] == 1 || 
				connections[k][connection_num] == -1 || k == connection_num)  // -1 means connection cannot be made due to going over connection limit
			{
				connection_num = rand() % 7;
			}

			connections[k][connection_num] = 1;
			connections[connection_num][k] = 1;

		} // end making connections

		// set the rest of the row in connections matrix to -1
		// signifies that its connection limit has been reached
		for (j = 0; j < 7; j++)
		{
			if (connections[k][j] == 0)
			{
				connections[k][j] = -1;	//signifies that this room has reached its connection limit
			}
		}
			
		/* Output room to files */
		output_file_ptr = fopen(file_path, "w");

		if (output_file_ptr == NULL)
		{
			fprintf(stderr, "Could not open %s\n", file_path);
			perror("in main");
			exit(1);
		}

		/* Print out rooms */
		fprintf(output_file_ptr, "ROOM NAME: %s\n\n", a_room[k].name);
		int n;
		int p = 1;
		for(n = 0; n < 7; n++)
		{
				if (connections[k][n] == 1)
				{
					fprintf(output_file_ptr, "CONNECTION %d: %s\n\n", p, a_room[n].name);
					p++;
				}
		}
		fprintf(output_file_ptr, "ROOM TYPE: %s\n\n", a_room[k].type);

		path_length = strlen(file_path) - 1;
		file_path[path_length] = '\0';

		fclose(output_file_ptr); 
	
	} //end of for loop to generate room connections and output rooms to files

} //end of makeRooms function

void playGame(char* directory)
{
	int no_of_moves = 0;
	char** path = NULL; //an array of cstrings to track the room path
	
	char* file_path = directory;
	char line[30];
	char file_num[2] = { '0', '\0' };
	FILE *fp;
	char curr_room[15];
	char* connection_room[25];
	char room_type[15];
	int line_length;
	char* connection_list;
	char next_room[30];
	char* room_file_arr[7];
	int dir_length;
	char temp[2];
	
	/* Read each file's room name and track associated room number. */
	int j;
	char j_str[2];
	for (j = 0; j < 7; j++)
	{
		sprintf(j_str, "%d", j);
		strcat(directory, j_str);

		fp = fopen(directory, "r"); // read mode

		if (fp == NULL)
		{
			perror("Error while opening the file.\n");
		}
		else
		{
			if (fgets(line, 50, fp) != NULL)
			{
				line_length = strlen(line);
				room_file_arr[j] = (char*)malloc(sizeof(char) * line_length);
				strncpy(room_file_arr[j], line + 11, 20);
				int str_length = strlen(room_file_arr[j]) - 1;
				room_file_arr[j][str_length] = '\0';	// get rid of \n character
			}

			fgets(temp, 50, fp);	// throw away extra newline
		}
		dir_length = strlen(directory) - 1;
		directory[dir_length] = '\0';
		fclose(fp);
	}
	
	/*REPEAT LOOP BEGINS HERE TO OUTPUT ROOM AND ITS CONNECTIONS*/
	/* Read start room and print to screen. */
	while (room_type != "END_ROOM")
	{
		strcat(directory, file_num);

		fp = fopen(directory, "r"); // read mode

		if (fp == NULL)
		{
			perror("Error while opening the file.\n");
		}
		else
		{
			if (fgets(line, 50, fp) != NULL)
			{
				fgets(temp, 50, fp);	//throw newline character away
				strncpy(curr_room, line + 11, 20);
			}
			int i = 0;
			for (i = 0; i < 25; i++)
			{
				connection_room[i] = NULL;
			}
			
			connection_list = (char*)malloc(sizeof(char) * 100);	//FIX MEMORY LEAK
			connection_list[0] = '\0';	//empty string
			i = 0;
			while (fgets(line, 50, fp) != NULL && line[0] == 'C')
			{
				fgets(temp, 50, fp);	//throw newline character away
				line_length = strlen(line);
				if (connection_room[i] != NULL)
				{
					free(connection_room[i]);
				}
				connection_room[i] = (char*)malloc(sizeof(char) * line_length);
				
				strncpy(connection_room[i], line + 14, line_length - 15);
				connection_room[i][line_length - 15] = '\0';
				
				strcat(connection_list, connection_room[i]);
				strcat(connection_list, ", ");
				i++;
			}
			line_length = strlen(connection_list);
			connection_list[line_length - 2] = '\0';
			
			fclose(fp);

			line_length = strlen(line);
			strncpy(room_type, line + 11, line_length - 11);
			line_length = strlen(room_type) - 1;
			room_type[line_length] = '\0';

			if (strncmp(room_type,"END_ROOM", 8) == 0)
			{
				printf("\n\nYOU HAVE FOUND THE END ROOM. CONGRATULATIONS!\n\n");
				printf("YOU TOOK %d STEPS. YOUR PATH TO VICTORY WAS:\n\n", no_of_moves);
				for (i = 0; i < no_of_moves; i++)
				{
					printf("%s\n\n", path[i]);
				}

				exit(0);
			}
			else
			{
				i = 7;
				while (i == 7)
				{
					printf("\nCURRENT LOCATION: ");
					fputs(curr_room, stdout);
					printf("\nPOSSIBLE CONNECTIONS: ");
					fputs(connection_list, stdout);
					printf("\n\n");
					printf("WHERE TO?> ");
					scanf("%s", next_room);
					int str_length = strlen(next_room);
					int j = 0;
					for (; j < 7; j++)
					{
						if (strcmp(next_room, room_file_arr[j]) != 0)
						{
							continue;
						}
						else
						{
							break;
						}
					}
					i = j;
					if (j == 7)
					{
						printf("\n\nHUH? I DON'T UNDERSTAND THAT ROOM. TRY AGAIN.\n\n");
					}
				}
			}
		}
				
		//if not end room, then get next room index and append to directory
		int j;
		for (j = 0; j < 7; j++)
		{
			if (strcmp(next_room, room_file_arr[j]) == 0)
			{
				char** temp;
				temp = path;
				no_of_moves++;
				path = (char**)malloc(sizeof(char*) * no_of_moves);
				assert(path != NULL);

				if (temp != NULL)
				{
					memcpy(path, temp, sizeof(char*) * (no_of_moves - 1));
					free(temp);
				}
				path[no_of_moves - 1] = room_file_arr[j];
				
				sprintf(file_num, "%d", j);

				dir_length = strlen(directory) - 1;
				directory[dir_length] = '\0';
			}
		}

		if (connection_list != NULL)
		{
			free(connection_list);
		}

	}//END BRACKET FOR GAME LOOP HERE

}//End of playGame function


