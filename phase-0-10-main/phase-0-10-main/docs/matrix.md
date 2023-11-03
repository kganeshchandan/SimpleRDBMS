# Page Layout
The default page layout which has already been implemented for the tables is in a row-wise fashion, that is a chunk of consecutive rows taken from the table from a page corresponding to the table. But Matrices have an additional constraint that the row size could be greater than the block size. So to store Matrices as pages, we need to store them as small submatrices. So a Matrix is stored as a collection of pages where each page is a square submatrix. Instead of this, we could have simply done row-wise storage as well, that is we can consider the matrix as a large 1D array instead of a 2D array, and pages will contain block-sized consecutive numbers from this  1D array. But this is will be inefficient when we calculate the transposes and perform cross transport. A matrix-type page will have fewer page reads than a row-wise page type while performing the cross transpose operation. Thus, keeping this in mind we go ahead with the idea of a square submatrix page type.

To implement this strategy we create a new class matrix, which is very analogous to the 'table` class that is already present. We do modifications to this new matrix class, especially the blockify() method inside the matrix class. The blockify will first read the .csv file line by line till it reaches a block_size value; once it reaches, it segments the stream read so far into pages, and buffermanager.writePage() is executed. This is done till we reach the end.  All the calculations performed also pad the matrix, so if the no of rwos is not a multiple of page size, then they are 0 padded.

# Cross Transpose
The logic is that once we have broken the matrix into small square submatrixes, we can transpose each page in our memory and then we can transpose these blocks across the diagonal. In this way, we can get the whole transpose.

We have two temporary blocks. Since we know that both the matrices have the same dimensions and pages, we iterate through the first matrix row-wise and 2nd matrix column-wise and for every block of these two temporary blocks we transpose each of these and place in their opposite pages, Hence we constant space of only those two pages,. Finally, the buffer pool is cleared. 

# Part 3   
If the given matrix has a lot of zeros, then by storing a large amount of repetitive 0s would be a huge waste of memory,

One approach to this would be that if the no of nonzeros are way too less, then it would be more efficient for us to store the matrix coordinates of wherever 1 is present. This would be an optimal way to store sparse matrixes. We can use a new class that stores the <x,y, value>.
The large matrixes can still be stored this way by simply storing the <x,y,value> object of the small page. <x,y> will be starting from the co-rodinate of the page itself.
Again if we want to do transpose calculations it becomes even more easier since we can simply swap the rownumber and the column number