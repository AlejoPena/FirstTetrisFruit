/*
CMPT 361 Assignment 1 - FruitTetris implementation Sample Skeleton Code

- This is ONLY a skeleton code showing:
How to use multiple buffers to store different objects
An efficient scheme to represent the grids and blocks

- Compile and Run:
Type make in terminal, then type ./FruitTetris



MODIFIED BY Alejandro Pena
 feb 17/2016
*/

#include "include/Angel.h"
#include <cstdlib>
#include <iostream>
#include <set>

using namespace std;

// xsize and ysize represent the window size - updated if window is reshaped to prevent stretching of the game
int xsize = 400; 
int ysize = 720;

// current tile
vec2 tile[4]; // An array of 4 2d vectors representing displacement from a 'center' piece of the tile, on the grid
vec2 tilepos = vec2(5, 19); // The position of the current tile using grid coordinates ((0,0) is the bottom left corner)
int tileX[4]; //stores the location of each piece of the tile, used for collision detection
int tileY[4];


// Arrays for storing all possible positions of each tile type
// The 'tile' array will always be some element [i][j] of this array (an array of vec2)
vec2 allRotationsshapes[16][4] = 
//vec2 allRotationsLshapes[12][4]
	{{vec2(-1,-1), vec2(-1, 0), vec2(0,0), vec2( 1, 0)},
	 {vec2( 1,-1), vec2( 0,-1), vec2(0,0), vec2( 0, 1)},     
	 {vec2( 1, 1), vec2( 1, 0), vec2(0,0), vec2(-1, 0)},  
	 {vec2(-1, 1), vec2( 0, 1), vec2(0,0), vec2( 0,-1)},

///vec2 allRotationsSshape[2][4] 
	{vec2(-1,-1), vec2(0,-1), vec2(0,0), vec2(1,0)},
	 {vec2( 1,-1), vec2(1, 0), vec2(0,0), vec2(0,1)},
	 //repetition to balance random choices
	 {vec2(-1,-1), vec2(0,-1), vec2(0,0), vec2(1,0)},
	 {vec2( 1,-1), vec2(1, 0), vec2(0,0), vec2(0,1)},

//vec2 allRotationsIshape[2][4] = 
	{vec2(-2, 0), vec2(-1, 0), vec2(0,0), vec2(1,0)},
	 {vec2( 0,-2), vec2( 0,-1), vec2(0,0), vec2(0,1)},
	 //repetition to balance random choices
	 {vec2(-2, 0), vec2(-1, 0), vec2(0,0), vec2(1,0)},
	 {vec2( 0,-2), vec2( 0,-1), vec2(0,0), vec2(0,1)},

//vec2 allRotationsTshape[4][4] = 
	{vec2(0, -1), vec2(0,0), vec2(0, 1), vec2(-1,0)},
	{vec2(1, 0), vec2(0, 0), vec2(-1,0), vec2(0, 1)},     
	{vec2(0, -1), vec2(0,0), vec2(0, 1), vec2(1,  0)},  
	{vec2(0,-1), vec2(0, 0), vec2(1, 0), vec2(-1, 0)}};


// colors
vec4 green  = vec4(0.3, 1.0, 0.0, 1.0);
vec4 red    = vec4(0.8, 0.0, 0.0, 1.0);
vec4 purple = vec4(0.9, 0.0, 1.0, 1.0);
vec4 yellow = vec4(1.0, 1.0, 0.0, 1.0);
vec4 orange = vec4(0.5, 1.0, 0.0, 1.0); 

vec4 white  = vec4(1.0, 1.0, 1.0, 1.0);
vec4 black  = vec4(0.0, 0.0, 0.0, 1.0);

vec4 colorList[5] = {purple, yellow, orange, green,red};
vec4 colors[4];//= {black, black, black, black};

//board[x][y] represents whether the cell (x,y) is occupied
bool board[10][20]; 

//An array containing the colour of each of the 10*20*2*3 vertices that make up the board
//Initially, all will be set to black. As tiles are placed, sets of 6 vertices (2 triangles; 1 square)
//will be set to the appropriate colour in this array before updating the corresponding VBO
vec4 boardcolours[1200];

// location of vertex attributes in the shader program
GLuint vPosition;
GLuint vColor;

// locations of uniform variables in shader program
GLuint locxsize;
GLuint locysize;

// VAO and VBO
GLuint vaoIDs[3]; // One VAO for each object: the grid, the board, the current piece
GLuint vboIDs[6]; // Two Vertex Buffer Objects for each VAO (specifying vertex positions and colours, respectively)

//-------------------------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------------------------

// When the current tile is moved or rotated (or created), update the VBO containing its vertex position data
void updatetile()
{
	// Bind the VBO containing current tile vertex positions
	glBindBuffer(GL_ARRAY_BUFFER, vboIDs[4]); 

	// For each of the 4 'cells' of the tile,
	for (int i = 0; i < 4; i++) 
	{
		// Calculate the grid coordinates of the cell
		GLfloat x = tilepos.x + tile[i].x; 
		GLfloat y = tilepos.y + tile[i].y;

		tileX[i] = (int)x;
		tileY[i] = (int)y;

		// Create the 4 corners of the square - these vertices are using location in pixels
		// These vertices are later converted by the vertex shader
		vec4 p1 = vec4(33.0 + (x * 33.0), 33.0 + (y * 33.0), .4, 1); 
		vec4 p2 = vec4(33.0 + (x * 33.0), 66.0 + (y * 33.0), .4, 1);
		vec4 p3 = vec4(66.0 + (x * 33.0), 33.0 + (y * 33.0), .4, 1);
		vec4 p4 = vec4(66.0 + (x * 33.0), 66.0 + (y * 33.0), .4, 1);

		// Two points are used by two triangles each
		vec4 newpoints[6] = {p1, p2, p3, p2, p3, p4}; 

		// Put new data in the VBO
		glBufferSubData(GL_ARRAY_BUFFER, i*6*sizeof(vec4), 6*sizeof(vec4), newpoints); 
	}

	glBindVertexArray(0);
}

//-------------------------------------------------------------------------------------------------------------------

// Called at the start of play and every time a tile is placed
void newtile()
{
		
		tilepos = vec2(5 , 19); // Put the tile at the top of the board

		// Update the geometry VBO of current tile
		int nextTileShape = rand() % 4; //randomly select one of the 3 tile shapes
		
			int orientationshape = rand() % 16;
			for (int i = 0; i < 4; i++){
				tile[i] = allRotationsshapes[orientationshape][i]; // Get the 4 pieces of the new tile
			}
		
		updatetile(); 

		// Update the color VBO of current tile
		vec4 newcolours[24];
		vec4 randColor; 
		//assign every 6 verticies with the same randomly generated color
		for (int i = 0; i < 4; i++)
		{
			randColor = colorList[rand() % 5];
			for (int j = 0; j < 6; j++)
			{
				newcolours[(i * 6) + j] = randColor;
							
			}
			colors[i] = randColor;
		}
		glBindBuffer(GL_ARRAY_BUFFER, vboIDs[5]); // Bind the VBO containing current tile vertex colours
		glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(newcolours), newcolours); // Put the colour data in the VBO
		glBindBuffer(GL_ARRAY_BUFFER, 0);
	
		glBindVertexArray(0);
	}


//-------------------------------------------------------------------------------------------------------------------

void initGrid()
{
	// ***Generate geometry data
	vec4 gridpoints[64]; // Array containing the 64 points of the 32 total lines to be later put in the VBO
	vec4 gridcolours[64]; // One colour per vertex
	// Vertical lines 
	for (int i = 0; i < 11; i++){
		gridpoints[2*i] = vec4((33.0 + (33.0 * i)), 33.0, 0, 1);
		gridpoints[2*i + 1] = vec4((33.0 + (33.0 * i)), 693.0, 0, 1);
		
	}
	// Horizontal lines
	for (int i = 0; i < 21; i++){
		gridpoints[22 + 2*i] = vec4(33.0, (33.0 + (33.0 * i)), 0, 1);
		gridpoints[22 + 2*i + 1] = vec4(363.0, (33.0 + (33.0 * i)), 0, 1);
	}
	// Make all grid lines white
	for (int i = 0; i < 64; i++)
		gridcolours[i] = white;


	// *** set up buffer objects
	// Set up first VAO (representing grid lines)
	glBindVertexArray(vaoIDs[0]); // Bind the first VAO
	glGenBuffers(2, vboIDs); // Create two Vertex Buffer Objects for this VAO (positions, colours)

	// Grid vertex positions
	glBindBuffer(GL_ARRAY_BUFFER, vboIDs[0]); // Bind the first grid VBO (vertex positions)
	glBufferData(GL_ARRAY_BUFFER, 64*sizeof(vec4), gridpoints, GL_STATIC_DRAW); // Put the grid points in the VBO
	glVertexAttribPointer(vPosition, 4, GL_FLOAT, GL_FALSE, 0, 0); 
	glEnableVertexAttribArray(vPosition); // Enable the attribute
	
	// Grid vertex colours
	glBindBuffer(GL_ARRAY_BUFFER, vboIDs[1]); // Bind the second grid VBO (vertex colours)
	glBufferData(GL_ARRAY_BUFFER, 64*sizeof(vec4), gridcolours, GL_STATIC_DRAW); // Put the grid colours in the VBO
	glVertexAttribPointer(vColor, 4, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(vColor); // Enable the attribute
}


void initBoard()
{
	// *** Generate the geometric data
	vec4 boardpoints[1200];
	for (int i = 0; i < 1200; i++)
		boardcolours[i] = black; // Let the empty cells on the board be black
	// Each cell is a square (2 triangles with 6 vertices)
	for (int i = 0; i < 20; i++){
		for (int j = 0; j < 10; j++)
		{		
			vec4 p1 = vec4(33.0 + (j * 33.0), 33.0 + (i * 33.0), .5, 1);
			vec4 p2 = vec4(33.0 + (j * 33.0), 66.0 + (i * 33.0), .5, 1);
			vec4 p3 = vec4(66.0 + (j * 33.0), 33.0 + (i * 33.0), .5, 1);
			vec4 p4 = vec4(66.0 + (j * 33.0), 66.0 + (i * 33.0), .5, 1);
			
			// Two points are reused
			boardpoints[6*(10*i + j)    ] = p1;
			boardpoints[6*(10*i + j) + 1] = p2;
			boardpoints[6*(10*i + j) + 2] = p3;
			boardpoints[6*(10*i + j) + 3] = p2;
			boardpoints[6*(10*i + j) + 4] = p3;
			boardpoints[6*(10*i + j) + 5] = p4;
		}
	}

	// Initially no cell is occupied
	for (int i = 0; i < 10; i++)
		for (int j = 0; j < 20; j++)
			board[i][j] = false; 


	// *** set up buffer objects
	glBindVertexArray(vaoIDs[1]);
	glGenBuffers(2, &vboIDs[2]);

	// Grid cell vertex positions
	glBindBuffer(GL_ARRAY_BUFFER, vboIDs[2]);
	glBufferData(GL_ARRAY_BUFFER, 1200*sizeof(vec4), boardpoints, GL_STATIC_DRAW);
	glVertexAttribPointer(vPosition, 4, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(vPosition);

	// Grid cell vertex colours
	glBindBuffer(GL_ARRAY_BUFFER, vboIDs[3]);
	glBufferData(GL_ARRAY_BUFFER, 1200*sizeof(vec4), boardcolours, GL_DYNAMIC_DRAW);
	glVertexAttribPointer(vColor, 4, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(vColor);
}

// No geometry for current tile initially
void initCurrentTile()
{
	glBindVertexArray(vaoIDs[2]);
	glGenBuffers(2, &vboIDs[4]);

	// Current tile vertex positions
	glBindBuffer(GL_ARRAY_BUFFER, vboIDs[4]);
	glBufferData(GL_ARRAY_BUFFER, 24*sizeof(vec4), NULL, GL_DYNAMIC_DRAW);
	glVertexAttribPointer(vPosition, 4, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(vPosition);

	// Current tile vertex colours
	glBindBuffer(GL_ARRAY_BUFFER, vboIDs[5]);
	glBufferData(GL_ARRAY_BUFFER, 24*sizeof(vec4), NULL, GL_DYNAMIC_DRAW);
	glVertexAttribPointer(vColor, 4, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(vColor);
}

void init()
{
	// Load shaders and use the shader program
	GLuint program = InitShader("vshader.glsl", "fshader.glsl");
	glUseProgram(program);

	// Get the location of the attributes (for glVertexAttribPointer() calls)
	vPosition = glGetAttribLocation(program, "vPosition");
	vColor = glGetAttribLocation(program, "vColor");

	// Create 3 Vertex Array Objects, each representing one 'object'. Store the names in array vaoIDs
	glGenVertexArrays(3, &vaoIDs[0]);

	// Initialize the grid, the board, and the current tile
	initGrid();
	initBoard();
	initCurrentTile();

	// The location of the uniform variables in the shader program
	locxsize = glGetUniformLocation(program, "xsize"); 
	locysize = glGetUniformLocation(program, "ysize");

	// Game initialization
	newtile(); // create new next tile

	// set to default
	glBindVertexArray(0);
	glClearColor(0, 0, 0, 0);
}

//-------------------------------------------------------------------------------------------------------------------

// Rotates the current tile, if there is room
void rotate()
{
	

	bool rotatable = true;
	vec2 rotateTile[4];


	for (int i = 0; i < 4; i++)
	{
 		GLfloat rotX = -1 * tile[i].y;
 		GLfloat rotY = tile[i].x;
 		GLfloat newPosX = tilepos.x + rotX;
 		GLfloat newPosY = tilepos.y + rotY;
		
		if (newPosX < 0 || newPosX > 9|| newPosY < 0 || board[(int)newPosX][(int)newPosY] == true) 
		//check if rotation puts us out of bounds or collides with a set tile
		{
			rotatable = false;
			break;
		}
		else
		{
			rotateTile[i] = vec2(rotX,rotY);
		}

	}

	if (rotatable)
	{
		for (int i = 0; i < 4; i++)
		{
			tile[i] = rotateTile[i];
		}
	}
}

//-------------------------------------------------------------------------------------------------------------------

void shuffle()
{
	//temporarily stores the first color of our tile to replace the last color
	vec4 tempColor = colors[0];
	//shift the colors over on the tile
	for (int i = 0; i < 3; i++)
	{
		colors[i] = colors[i+1];
	}
	colors[3] = tempColor;

	// Update the color VBO of current tile
	vec4 newcolours[24];
	//assign every 6 verticies with the new color
	for (int i = 0; i < 4; i++)
	{
		for (int j = 0; j < 6; j++)
		{
			newcolours[(i * 6) + j] = colors[i];
						
		}
	}
	glBindBuffer(GL_ARRAY_BUFFER, vboIDs[5]); // Bind the VBO containing current tile vertex colours
	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(newcolours), newcolours); // Put the colour data in the VBO
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glBindVertexArray(0);
}





//-------------------------------------------------------------------------------------------------------------------
void bringdown(int col, int row)
{
	
		for (int yi = row; yi < 20; yi++)
		{
			for (int j = 0; j < 6; j++)
			{
				//top square becomes black, all other squares take on the colors of the square above
				if (yi == 19)
				{
					boardcolours[1140 + (col*6)+j] = black;
				}
				else
				{
					boardcolours[(yi * 60) + (col*6)+j] = boardcolours[(yi * 60) + (col*6)+j + 60]; 
				}
			}
			
			
				//switch lower sq for top one
				if (yi == 19)
				{
					board[col][19] = false;
				}
				else
				{
					board[col][yi] = board[col][yi+1];
				}
			
		}
		
}

// Checks if the specified row (0 is the bottom 19 the top) is full
// If every cell in the row is occupied, it will clear that cell and everything above it will shift down one row
/*void checkcrosstrio(int col, int row)
{    
		        //check one up
	
        glBindBuffer(GL_ARRAY_BUFFER, vboIDs[3]);
			if (row<19 && boardcolours[col+row*60].x == boardcolours[col+(row+1)*60].x)
			{   //check two up

				if(row < 18 && boardcolours[col+row*60] == boardcolours[col+(row+2)*60])
				{ //check vertical colors
                 	for(int i=0;i<3; i++)
                 	{
				  		bringdown(col,row);
				 	}
					//check one down
				}else {
					if(boardcolours[col+row*60] == boardcolours[col+(row-1)*60])
					{

						for(int i=0;i<3; i++)
	                 	{
					  		bringdown(col,row-1);
					 	}
					}

				}

			}else if(row>0 && boardcolours[col*10+row*20].x == boardcolours[col+(row-1)*60])
			{
				if(row > 1 && boardcolours[col+row*60] == boardcolours[col+(row-2)*60])
				{ //check vertical colors

					for(int i=0;i<3; i++)
                 	{
				  		bringdown(col,row-2);
				 	}
					
                  //horizontal
				}else{
						if(col>0 && boardcolours[col+row*60] == boardcolours[(col-1)+row*60])
						{
							if(col > 1 && boardcolours[col+row*60] == boardcolours[(col-2)+row*60])
							{
								for(int i=0;i<3; i++)
	                 			{
					  				bringdown(col+i-2,row);
					 			}

							}else{
								if(col>0 && boardcolours[col+row*60] == boardcolours[(col+1)+row*60])
								{
									for(int i=0;i<3;i++)
			             			{
						  				bringdown(col+i-1,row);
						 			}
						       }
						    }
					
						}else{
							if(col<9 && boardcolours[col+row*60] == boardcolours[(col+1)+row*60])
							{
								if(col<8 && boardcolours[col+row*60] == boardcolours[(col+2)+row*60])
								{
									for(int i=0;i<3;i++)
			             			{
						  				bringdown(col+i,row);
						 			}
								}
							}			
						}
					}
			}
		glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(boardcolours), boardcolours);
		glBindVertexArray(0);

	
}*/


void checkall(int col, int row){
	 glBindBuffer(GL_ARRAY_BUFFER, vboIDs[3]);

             cout << "color: " <<  boardcolours[col*6+row*60].x << endl;

	        //cross  vertical
			if (row<18 && boardcolours[col*6+row*60].x == boardcolours[col*6+(row+1)*60].x
				       && boardcolours[col*6+row*60].x == boardcolours[col*6+(row+2)*60].x)
			{    
				for(int i=0;i<3; i++)
                 	{
				  		bringdown(col,row);
				 	}

			} if(row<19 && row > 0 && boardcolours[col*6+row*60].x == boardcolours[col*6+(row+1)*60].x
				                   && boardcolours[col*6+row*60].x == boardcolours[col*6+(row-1)*60].x)
			{
				for(int i=0;i<3; i++)
                 	{
				  		bringdown(col,row-1);
				 	}
			}
			if(row > 1 && boardcolours[col*6+row*60].x == boardcolours[col*6+(row-1)*60].x
				            && row<19 && boardcolours[col*6+row*60].x == boardcolours[col*6+(row-2)*60].x)
				{
					for(int i=0;i<3; i++)
	                 	{
					  		bringdown(col,row-2);
					 	}
				 }
			//cross horizontal 
			if(col>1 && boardcolours[col*6+row*60].x == boardcolours[(col-1)*6+row*60].x
					          && boardcolours[col*6+row*60].x == boardcolours[(col-2)*6+row*60].x)
				{
					for(int i=0;i<3; i++)
	                 	{
					  		bringdown(col-i,row);
					 	}
				}
			if(col>0 && col<9 && boardcolours[col*6+row*60].x == boardcolours[(col-1)*6+row*60].x
					          && boardcolours[col*6+row*60].x == boardcolours[(col+1)*6+row*60].x)
			{
				for(int i=0;i<3; i++)
	                 	{
					  		bringdown(col+i-1,row);
					 	}
			}
			if(col<8 && boardcolours[col*6+row*60].x == boardcolours[(col-1)*6+row*60].x
					          && boardcolours[col*6+row*20].x == boardcolours[(col-2)*6+row*60].x)
			{
				for(int i=0;i<3; i++)
	                 	{
					  		bringdown(col-i,row);
					 	}
			}

			// three diag  up left
			 if(row<18 && col>1 && boardcolours[col*6+row*60].x == boardcolours[(col-1)*6+(row+1)*60].x
				      && boardcolours[col*6+row*60].x == boardcolours[(col-1)*6+(row+2)*60].x)
			{    
				for(int i=0;i<3; i++)
                 	{
				  		bringdown(col-i,row+i);
				 	}

			}if(row<19 && row > 0 && boardcolours[col*6+row*60].x == boardcolours[(col-1)*6+(row+1)*60].x
				            && row<19 && boardcolours[col*6+row*60].x == boardcolours[(col+1)*6+(row-1)*60].x)
			{
				for(int i=0;i<3; i++)
                 	{
				  		bringdown(col-i-1,row+i-1);
				 	}
			}
			if(row > 1 && col < 8 && boardcolours[col*6+row*60].x == boardcolours[(col+1)*6+(row-1)*60].x
				            && row<19 && boardcolours[col*6+row*60].x == boardcolours[(col+2)*6+(row-2)*60].x)
				{
				 for(int i=0;i<3; i++)
                 	{
				  		bringdown(col+i,row-i);
				 	}
				 }
			//diag right up
			if(row<18 && col < 8 && boardcolours[col*6+row*60].x == boardcolours[(col+1)*6+(row+1)*60].x
					          && boardcolours[col*6+row*60].x == boardcolours[(col+2)*6+(row+2)*60].x)
				{
					for(int i=0;i<3; i++)
                 	{
				  		bringdown(col+i,row+i);
				 	}

				}
			if(row<19 && col < 9 && boardcolours[col*6+row*60].x == boardcolours[(col+1)*6+(row+1)*60].x
					          && boardcolours[col*6+row*60].x == boardcolours[(col-1)*6+(row-1)*60].x)
			{
				for(int i=0;i<3; i++)
                 	{
				  		bringdown(col-i+1,row-i+1);
				 	}

			}
			if(col>1 && row >1 && boardcolours[col*6+row*60].x == boardcolours[(col-1)*6+(row-1)*60].x
					          && boardcolours[col*6+row*60].x == boardcolours[(col-2)*6+(row-2)*60].x)
			{
			for(int i=0;i<3; i++)
                 	{
				  		bringdown(col-i,row-i);
				 	}
			}
		glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(boardcolours), boardcolours);
		glBindVertexArray(0);

			


}

//-------------------------------------------------------------------------------------------------------------------

// Checks if the specified row (0 is the bottom 19 the top) is full
// If every cell in the row is occupied, it will clear that cell and everything above it will shift down one row
void checkfullrow(int row)
{
	bool occupied = true;
	for (int i = 0; i < 10; i++)
	{
		occupied = occupied && board[i][row];
	}
	if (occupied)
	{
		glBindBuffer(GL_ARRAY_BUFFER, vboIDs[3]);

		//loop begins at the row which is cleared
		for (int i = 0; i < 10; i++)
		{
			bringdown(i,row);
		}

		glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(boardcolours), boardcolours);
		glBindVertexArray(0);
	}

}

//-------------------------------------------------------------------------------------------------------------------

// Places the current tile - update the board vertex colour VBO and the array maintaining occupied cells
void settile()
{
	//update VBO color
	glBindBuffer(GL_ARRAY_BUFFER, vboIDs[3]);
	std::set<int, std::greater<int> > rowsAffected;
	std::set<int>::iterator it;
	for (int i = 0; i < 4; i++)
	{
		//calculate grid coorindates for each cell	
		GLuint x = tilepos.x + tile[i].x; 
		GLuint y = tilepos.y + tile[i].y;

		//cout << "x :" << x <<" y:" << y << endl;

		//update the corresponding cell to get the same color as the tile
		for (int j = 0; j < 6; j++)
		{
			boardcolours[(y*60)+(x*6) + j] = colors[i];
		}

		board[x][y] = true;

		rowsAffected.insert(y);
	}
	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(boardcolours), boardcolours);
	glBindVertexArray(0);


	for (it = rowsAffected.begin(); it != rowsAffected.end(); ++it)
	{
		checkfullrow(*it);
	}

	for (int i = 0; i < 4; i++)
	{
		//calculate grid coorindates for each cell	
		GLuint x = tilepos.x + tile[i].x; 
		GLuint y = tilepos.y + tile[i].y;
		cout << "x:" << x << " y:"<< y<< endl;

		checkall(x,y);

	}


}

//-------------------------------------------------------------------------------------------------------------------

// Given (x,y), tries to move the tile x squares to the right and y squares down
// Returns true if the tile was successfully moved, or false if there was some issue
bool movetile(vec2 direction)
{
	return false;
}
//-------------------------------------------------------------------------------------------------------------------

//forces the tile to move down on its own
void fallingtile(int data)
{
	//halt falling bricks if starting point is obstructed
	if (!board[5][19] && !board[5][18] && !board[4][19] && !board[4][18] && !board[6][19] && !board[6][18] )
	
	{
		tilepos.y -= 1;
		updatetile();

		glutPostRedisplay();
		glutTimerFunc(400, fallingtile, 0);
	}
}


//-------------------------------------------------------------------------------------------------------------------

// Starts the game over - empties the board, creates new tiles, resets line counters
void restart()
{
	//initGrid();
	initBoard();
	//initCurrentTile();
	newtile();
	glutTimerFunc(400, fallingtile, 0);
}
//-------------------------------------------------------------------------------------------------------------------

// Draws the game
void display()
{

	glClear(GL_COLOR_BUFFER_BIT);

	glUniform1i(locxsize, xsize); // x and y sizes are passed to the shader program to maintain shape of the vertices on screen
	glUniform1i(locysize, ysize);

	glBindVertexArray(vaoIDs[1]); // Bind the VAO representing the grid cells (to be drawn first)
	glDrawArrays(GL_TRIANGLES, 0, 1200); // Draw the board (10*20*2 = 400 triangles)

	glBindVertexArray(vaoIDs[2]); // Bind the VAO representing the current tile (to be drawn on top of the board)
	glDrawArrays(GL_TRIANGLES, 0, 24); // Draw the current tile (8 triangles)

	glBindVertexArray(vaoIDs[0]); // Bind the VAO representing the grid lines (to be drawn on top of everything else)
	glDrawArrays(GL_LINES, 0, 64); // Draw the grid lines (21+11 = 32 lines)


	glutSwapBuffers();
}

//-------------------------------------------------------------------------------------------------------------------

// Reshape callback will simply change xsize and ysize variables, which are passed to the vertex shader
// to keep the game the same from stretching if the window is stretched
void reshape(GLsizei w, GLsizei h)
{
	xsize = w;
	ysize = h;
	glViewport(0, 0, w, h);
}

//-------------------------------------------------------------------------------------------------------------------

// Handle arrow key keypresses
void special(int key, int x, int y)
{
	//halt arrow key commands if starting point is obstructed
	if (!board[5][19] && !board[5][18] && !board[4][19] && !board[4][18] && !board[6][19] && !board[6][18] )
	
	{
		bool movable = true;
		
		switch(key) {
		case GLUT_KEY_UP :
			rotate();
			break;
		case GLUT_KEY_DOWN :
			for (int i = 0; i < 4; i++)
			{
				if (board[tileX[i]][tileY[i]-1] == true)
				{
					movable = false;
					break;
				}			
			}
			if (movable)
			{
				tilepos.y -= 1;
			}
			break;
		case GLUT_KEY_LEFT :
			for (int i = 0; i < 4; i++)
			{
				if (tileX[i] == 0  || board[tileX[i]-1][tileY[i]] == true)
				{
					movable = false;
					break;
				}			
			}
			if (movable)
			{
				tilepos.x -= 1;
			}			
			break;
		case GLUT_KEY_RIGHT:
			for (int i = 0; i < 4; i++)
			{
				if (tileX[i] == 9 || board[tileX[i]+1][tileY[i]] == true)
				{
					movable = false;
					break;
				}		
			}
			if (movable)
			{
				tilepos.x += 1;
			}	
			
			break;
		}
		updatetile();

		glutPostRedisplay();
	}
}

//-------------------------------------------------------------------------------------------------------------------

// Handles standard keypresses
void keyboard(unsigned char key, int x, int y)
{
	switch(key) 
	{
		case 033: // Both escape key and 'q' cause the game to exit
		    exit(EXIT_SUCCESS);
		    break;
		case 'q':
			exit (EXIT_SUCCESS);
			break;
		case 'r': // 'r' key restarts the game
			restart();
			break;
		case ' ':
			shuffle();
			break;
	}
	//glutPostRedisplay();
}

//-------------------------------------------------------------------------------------------------------------------

void idle(void)
{
	//loop through each piece of the tile to check for collision with the floor or a set tile
	for (int i = 0; i < 4; i++) 
	{
		//if the tile reaches the floor or the cell directly below is occupied		
		if ((tileY[i]) == 0 || board[tileX[i]][tileY[i]-1] == true) 

		{
			settile();
			if(board[5][19]){
			 //restart();
				break;
			}
			newtile();
			break;
		}
	}
	glutPostRedisplay();
}

//-------------------------------------------------------------------------------------------------------------------

int main(int argc, char **argv)
{
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE);
	glutInitWindowSize(xsize, ysize);
	glutInitWindowPosition(680, 178); // Center the game window (well, on a 1920x1080 display)
	glutCreateWindow("Fruit Tetris");
	glewInit();
	init();

	// Callback functions
	glutDisplayFunc(display);
	glutReshapeFunc(reshape);
	glutSpecialFunc(special);
	glutKeyboardFunc(keyboard);
	glutIdleFunc(idle);
	glutTimerFunc(400, fallingtile, 0);

	glutMainLoop(); // Start main loop
	return 0;
}
