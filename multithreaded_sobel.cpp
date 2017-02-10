#include <iostream>
#include <fstream>
#include <sstream>
#include <cstring>
#include <thread>
#include <mutex>

using namespace std;

mutex row_lock;

void write_image(int** array, int col, int row, string pathname)
{
	ofstream output_file(pathname);

	// save output buffer
	streambuf* cout_buffer = cout.rdbuf();
	
	// set stdout to the given output file
	cout.rdbuf(output_file.rdbuf());

	
	cout << "P2" << endl;
	cout << col << " " << row << endl;
	cout << 255 << endl;


	for (int i = 0; i < row; i++)
	{
		for (int j = 0; j < col; j++)
		{
			cout << array[i][j] << " ";
		}
		cout << endl;
	}

	// set output back to stdout
	cout.rdbuf(cout_buffer);

	output_file.close();
}


void sobel(int** input_2d_array, int height, int width, int** output_image_array, int start, int end)
{
	/* Algorithm provided by Professor Alex Veidenbaum */

	int maskX[3][3];
	int maskY[3][3];
	
	maskX[0][0] = -1; maskX[0][1] = 0; maskX[0][2] = 1;
	maskX[1][0] = -2; maskX[1][1] = 0; maskX[1][2] = 2;
	maskX[2][0] = -1; maskX[2][1] = 0; maskX[2][2] = 1;

	maskY[0][0] = 1; maskY[0][1] = 2; maskY[0][2] = 1;
	maskY[1][0] = 0; maskY[1][1] = 0; maskY[1][2] = 0;
	maskY[2][0] = -1; maskY[2][1] = -2; maskY[2][2] = -1;

	for (int x = start; x < end; x++)
	{
		for (int y = 0; y < width; y++)
		{
			int sumx = 0;
			int sumy = 0;

			int sum = 0; // test
			
			if (x == 0 || x == (height-1) || y == 0 || y == (width-1))
				sum = 0;
			
			else
			{
				for (int i = -1; i <= 1; i++ )
				{
					for (int j = -1; j<= 1; j++)
					{
						sumx += (input_2d_array[x+i][y+j] * maskX[i+1][j+1]);
					}
				}
				for (int i = -1; i <= 1; i++)
				{
					for (int j = -1; j <= 1; j++)
					{
						sumy += (input_2d_array[x+i][y+j] * maskY[i+1][j+1]);
					}
				}
				
				sum = (abs(sumx) + abs(sumy));
			}
	
			if (sum < 0)
				sum = 0;

			if (sum > 255)
				sum = 255;

			output_image_array[x][y] = (sum);
		
		}
	}	
}


// inspired by pseudo code
int get_dynamic_chunk(int* row_placement, int chunk_size, int max_row)
{
	// lock i		
	row_lock.lock();	
	
	if (*row_placement >= max_row)
		*row_placement -= chunk_size; // past the image boundaries; reset to before the boundary

	int end = (*row_placement + chunk_size);

	// Update the row_placement for next thread
	*(row_placement) = end;

	// unlock row_placement for next thread to execute above critical section 
	row_lock.unlock();

	return end; // the dynamic "chunk"
}


// inspired by pseudo code
void compute_chunk(int* row_placement, int chunk_size, int max_row, int col, int** input_2d_array, int** output_image_array)
{
	while (*row_placement < max_row) // don't allow thread to stop until the image array is completely written
	{
		int start = *(row_placement); // starting output row
		int end = get_dynamic_chunk(row_placement, chunk_size, max_row); // ending output row

		if (end > max_row)
			end = max_row + 1; // so that we include the last row

		if (start < max_row && end <= max_row) // make sure no threads write to junk values past output array
			sobel(input_2d_array, max_row, col, output_image_array, start, end); // write values to output image array
	}
}


int main(int argc, char* argv[])
{	
	string given_path = argv[1];
	string output_path = argv[2];

	int thread_count = atoi(argv[3]);

	thread thread_array[thread_count];
	int chunk_size = atoi(argv[4]);

	ifstream file(given_path);
	string buffer = "";
	int row, col;

	getline(file, buffer); // P2 comment is here	
	getline(file, buffer); // could be a comment OR size of image in pixels

	if (buffer.find("#") != std::string::npos) // found the comment line
		getline(file, buffer); // NOW buffer holds the dimension line.

	// buffer now contains "row col" dimensions of the PMG image.
	stringstream stream(buffer);

	// set the amount of cols for the 2d array
	stream >> col;

	// set the amount of rows for the 2d array
	stream >> row;

	if (chunk_size > row)
		chunk_size = row;

	if (chunk_size < 1)
		chunk_size = 1;

	// now get the max shade range of the PGM image.
	getline(file, buffer);

	int** input_2d_array = new int*[row];
	int** output_image_array = new int*[row];

	stringstream stream2;

	// leave string stream where the "pixels" are in the PMG image.
	stream2 << file.rdbuf(); 
	
	for (int i = 0; i < row; i++)
	{
		input_2d_array[i] = new int[col];
		output_image_array[i] = new int[col];

		for (int j = 0; j < col; j++)
		{
			stream2 >> input_2d_array[i][j];
		}
	}

	// The function call used to just run on main thread; need to chop up
	// sobel(input_2d_array, row, col, output_image_array);

	// Necessary
	int row_placement = 0;

	// thread create loop
	for (int i = 0; i < thread_count; i++)
		thread_array[i] = thread(compute_chunk, &row_placement, chunk_size, row, col, input_2d_array, output_image_array);

	// join loop
	for (int i = 0; i < thread_count; i++)
		thread_array[i].join();

	// main thread write 
	write_image(output_image_array, col, row, output_path);

	file.close();	

	delete input_2d_array;
	delete output_image_array;

	return 0;
}
