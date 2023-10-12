#include <db_cxx.h>
#include <string>
#include <assert.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include<sys/time.h>

using namespace std;

struct datablock
  {
    int blockinfo[5];
    double **blockdata; 
  };//Data Structure to be stored in to berkeleyDB as a value

class BerkeleyDB
{
 	private:
  	 Db db;
  	 string dbFileName;
 	public:
  	 BerkeleyDB(const string &path, const string &dbName,u_int32_t cflags = 0);
  	 ~BerkeleyDB()
   	 {
   	 } 
  	 void close();

    	template <class T> int insert( size_t &k, T d,int noof_block_rows,int noof_block_columns)//Method for Storing data in to berkeleyDB
    	{ 
      		int i,j;
      		size_t dsize =sizeof(d);
      		struct datablock temp;
      
       		temp.blockdata=(double **)malloc(noof_block_rows * sizeof(double *) );
       		for(i=0;i<noof_block_rows;i++)
       		temp.blockdata[i]=(double *)malloc(noof_block_columns * sizeof(double) );
          
       		for(i = 0; i <noof_block_rows; i++) 
       		{
         		for(j = 0; j <noof_block_columns; j++)
         		{
          			temp.blockdata[i][j]=d.blockdata[i][j];
         		}
       		}
      
       		for(i = 0;i<5;i++)
       		{
        		temp.blockinfo[i]=d.blockinfo[i];
       		}
        
      		Dbt key((void*)&k, sizeof( size_t) );
      		Dbt data((void*)&temp, dsize );
      		int stat = db.put(0, &key, &data, 0);
      		return stat; 
    	}

    	template <class T> T retrieve( size_t &k, T d)//Method for Retrieving data from berkeleyDB
    	{ 
      		Dbt key((void*)&k, sizeof(size_t));
      		Dbt dbtdata;
      		int stat = db.get( 0, &key, &dbtdata, 0);
      		size_t dsize = dbtdata.get_size();
      		if(dsize) 
      		{
			memcpy(&d,dbtdata.get_data(),dsize);
      		}
      		return d;
    	}
    
};

inline BerkeleyDB::BerkeleyDB(const string &path, const string &dbName,u_int32_t cflags) : db(0, 0), dbFileName(path + dbName)//Method for Creating & Opening the database file  
{
  	try
    	{
      
      		db.set_error_stream(&std::cerr);

      		db.set_pagesize(2048);
      		db.set_cachesize(0,64*1024, 0);
      		cout << dbFileName<< endl;
      
      		db.open( 0, dbFileName.c_str(), 0, DB_HASH, cflags, 0);
    	}
 	catch(DbException &e)
    	{
      		cerr << "Error opening database: " << dbFileName << "\n";
      		cerr << e.what() << std::endl;
    	}
  	catch(std::exception &e)
    	{
      		cerr << "Error opening database: " << dbFileName << "\n";
      		cerr << e.what() << std::endl;
    	}
}

inline void BerkeleyDB::close()//Method for Closing the database file
{
  	try
    	{
      		db.close(0);
      		cout <<"\n" << "Database " << dbFileName << " is closed." << endl;
    	}
  	catch(DbException &e)
    	{
      		cerr << "Error closing database: " << dbFileName << "\n";
      		cerr << e.what() << std::endl;
    	}
  	catch(std::exception &e)
    	{
      		cerr << "Error closing database: " << dbFileName << "\n";
      		cerr << e.what() << std::endl;
    	}
}

class LargeMatrix
{
 	private:
  	 mutable BerkeleyDB  *db;
   	 string pathname, dbname;
  	 struct datablock rbuf;
        public:
  	 LargeMatrix(const string &pname, const string &dname) 
  	 { 
    		pathname = pname; 
    		dbname = dname; 
  	 }
  	 ~LargeMatrix()
   	 {
    	 }

  	void openbdb( const int mode = DB_CREATE) const //Method for calling database creation function 
  	{
       		db = new BerkeleyDB(pathname, dbname, mode);
  	}
  	void closedb()//Method for calling database closing function 
  	{
       		if( db )
       		db->close();  
       		db = 0;
       		delete db;
  	}

	void setVal(int c, struct datablock blk,int noof_block_rows,int noof_block_columns)//Method for calling data insertion function
        {
        	size_t index =c;
   	        db->insert(index,blk,noof_block_rows,noof_block_columns);
        }

	struct datablock getVal(int c)//Method for calling data retrieving function 
        {
                size_t index =c; 
                rbuf=db->retrieve(index,rbuf);
                return rbuf;
        }

	void write_matrix(char matname,int noof_rows,int noof_columns,int noof_block_rows,int noof_block_columns);
 
	void read_matrix(double **blockdata,int noof_rows,int noof_columns,int noof_block_rows,int noof_block_columns);
 
	void print_matrix(double **blockdata,int noof_rows,int noof_columns);
 
};

inline void LargeMatrix::write_matrix(char matname,int noof_rows,int noof_columns,int noof_block_rows,int noof_block_columns)//Method used to store matrix block by block in to database
{
   	int gi,gj,i,j,k,nb=0;
   	struct datablock bloc;
  
     	bloc.blockdata=(double **)malloc(noof_block_rows * sizeof(double *) );
     	for(i=0;i<noof_block_rows;i++)
     	bloc.blockdata[i]=(double *)malloc(noof_block_columns * sizeof(double) );
    
     	for(gi=0; gi<noof_rows; gi=gi+noof_block_rows) 
     	{
       		for(gj=0; gj<noof_columns; gj=gj+noof_block_columns)
       		{
         		bloc.blockinfo[1]=gi;bloc.blockinfo[2]=gj;
         		bloc.blockinfo[3]=gi+(noof_block_rows-1);bloc.blockinfo[4]=gj+(noof_block_columns-1);
                        if(matname=='A')
                        {
         			for(i=0; i<noof_block_rows; i++) 
         			{ 
           				k=gj;
           				for(j=0; j<noof_block_columns; j++)
           				{  
             					bloc.blockdata[i][j]=k+1;
             					k++;
           				}
         			}
 		
         			nb++;
         			bloc.blockinfo[0]=nb;
         			setVal(nb,bloc,noof_block_rows,noof_block_columns);
			}
                        else if(matname=='B')    
			{
                        	for(i=0; i<noof_block_rows; i++) 
         			{ 
           				k=gj;
           				for(j=0; j<noof_block_columns; j++)
           				{  
             					bloc.blockdata[i][j]=k+1;
             					k++;
           				}
         			}
 		
         			nb++;
         			bloc.blockinfo[0]=nb;
         			setVal(nb,bloc,noof_block_rows,noof_block_columns);
			}
                        else
			{
                        	for(i=0; i<noof_block_rows; i++) 
         			{ 
           				for(j=0; j<noof_block_columns; j++)
           				{  
             					bloc.blockdata[i][j]=0;
           				}
         			}
 		
         			nb++;
         			bloc.blockinfo[0]=nb;
         			setVal(nb,bloc,noof_block_rows,noof_block_columns);
			} 
       		}
     	}
}

inline void LargeMatrix::read_matrix(double **blockdata,int noof_rows,int noof_columns,int noof_block_rows,int noof_block_columns)//Method used to Retrieve matrix block by block from database
{ 
   	int bj,bi,i,j,gi,gj,c=0,f;
       
   	struct datablock bloc;
       
     	for(gi=0; gi<noof_rows; gi=gi+noof_block_rows) 
     	{
       		for(gj=0; gj<noof_columns; gj=gj+noof_block_columns)
       		{
         		bi=gi;
         		bj=gj;
         		c++;
         		bloc=getVal(c);

         		for(i=0; i<noof_block_rows; i++) 
         		{ 
          			 for(j=0; j<noof_block_columns; j++)
           			 {  
             				blockdata[bi][bj]=bloc.blockdata[i][j];
             				bj++;
           			 }
           			 bi++;
           			 bj=gj;
         		} 
        	}
      	}
}

inline void LargeMatrix::print_matrix(double **blockdata,int noof_rows,int noof_columns)//Method used to print the matrix
{ 
   	for(int i=0; i<noof_rows; i++)
   	{
     		for(int j=0; j<noof_columns;j++)
     		{
       			cout<<blockdata[i][j]<<" ";
     		}
     		cout<<"\n";
   	}
}

LargeMatrix* open_matrix( const string &dbpathname,const string &dbname)
{
  	LargeMatrix *adb = new LargeMatrix(dbpathname, dbname);
  	adb->openbdb(DB_CREATE);
  	return adb;
}

int main(int argc, char **argv)
{
  	double **ablockdata,**bblockdata,**cblockdata;
  	int noof_rows_in_block_a,noof_columns_in_block_a,noof_rows_in_block_b,noof_columns_in_block_b;
  	if( argc != 8 )
  	{
    		cout<<"USAGE: ./[OBJECT-FILE] [MATRIX-A-ROWS] [MATRIX-A-COLUMNS] [MATRIX-B-ROWS] [MATRIX-B-COLUMNS] ";
    		cout<<"[MATRIX-A-BLOCKS-PER-ROW] [MATRIX-B-BLOCKS-PER-ROW] ";
    		cout<<"[DBPATH] \n";
    		exit(-1); 
  	} 
  
  	size_t  noof_a_rows = atoi( argv[1] );
  	size_t  noof_a_columns = atoi( argv[2] );
  
  	size_t  noof_b_rows = atoi( argv[3] );
  	size_t  noof_b_columns = atoi( argv[4] );
 
  	size_t  noof_a_row_blocks=atoi( argv[5] );
	size_t  noof_a_column_blocks=1;

  
  	size_t  noof_b_row_blocks=1;
	size_t  noof_b_column_blocks=atoi( argv[6] );
  
  	string  dbpath = argv[7];
        
        /*
		DESCRIPTION OF VARIABLES USED 
		
		noof_a_rows		: USED TO STORE MATRIX-A ROWS
		noof_a_columns		: USED TO STORE MATRIX-A COLUMNS        
		noof_b_rows		: USED TO STORE MATRIX-B ROWS
		noof_b_columns		: USED TO STORE MATRIX-B COLUMNS
		noof_a_row_blocks	: USED TO STORE NO OF BLOCKS IN A ROW OF MATRIX-A
		noof_a_column_blocks	: USED TO STORE NO OF BLOCKS IN A COLUMN OF MATRIX-A
		noof_b_row_blocks	: USED TO STORE NO OF BLOCKS IN A ROW OF MATRIX-B
		noof_b_column_blocks	: USED TO STORE NO OF BLOCKS IN A COLUMN OF MATRIX-B 
		dbpath			: USED TO STORE DATABASE PATH GIVEN IN COMMAND LINE 
		noof_rows_in_block_a	: USED TO STORE NO OF ROWS IN A BLOCK OF MATRIX-A
		noof_columns_in_block_a	: USED TO STORE NO OF COLUMNS IN A BLOCK OF MATRIX-A
		noof_rows_in_block_B	: USED TO STORE NO OF ROWS IN A BLOCK OF MATRIX-B
		noof_columns_in_block_B	: USED TO STORE NO OF COLUMNS IN A BLOCK OF MATRIX-B
 
        */        

  	if(noof_a_columns!= noof_b_rows)//Checking whether matrices can be multiplied or not
  	{
   		cout<<"\n Matrices cannot be multiplide no of columns in 1st matrix not equal to no of rows in 2nd matrix \n";
   		exit(-1);
  	}
  	if(noof_a_rows!= noof_a_columns)//Checking whether appropriate block size is given or not
  	{
    		if(noof_a_rows != noof_a_row_blocks || noof_b_columns != noof_b_column_blocks)
    		{
      			cout<<"\n Matrices cannot be multiplied because of uneven block size \n";
      			exit(-1);
    		}
  	} 
  	if((noof_a_rows % noof_a_row_blocks)==0 && (noof_a_columns % noof_a_column_blocks)==0)//Assigning no of rows and columns per block for matrix-a
  	{
   		noof_rows_in_block_a=noof_a_rows/noof_a_row_blocks;
   		noof_columns_in_block_a=noof_a_columns/noof_a_column_blocks;
  	}
  	else
  	{
   		cout<<"\n give the dividable block size for matrix A \n";
   		exit(-1);
  	}
  	if((noof_b_rows % noof_b_row_blocks)==0 && (noof_b_columns % noof_b_column_blocks)==0)//Assigning no of rows and columns per block for matrix-b
  	{
   		noof_rows_in_block_b=noof_b_rows/noof_b_row_blocks;
   		noof_columns_in_block_b=noof_b_columns/noof_b_column_blocks;
  	}
  	else
  	{
   		cout<<"\n give the dividable block size for matrix B \n";
   		exit(-1);
  	}
  	cout <<" matrix_A_size:" <<noof_a_rows<<"*"<<noof_a_columns<<endl;
  	cout <<" matrix_B_size:" <<noof_b_rows<<"*"<<noof_b_columns<<endl;
  	cout <<" matrix_A_blocksize :" <<noof_rows_in_block_a<<"*"<<noof_columns_in_block_a<<endl;
  	cout <<" matrix_B_blocksize :" <<noof_rows_in_block_b<<"*"<<noof_columns_in_block_b<<endl;
  	cout <<" dbpath:" << dbpath << endl; 
  
  	int global_i,global_j,global_k,i,j,k,a_block_no=0,b_block_no=0,c_block_no=0,change_c_blk_no=0,t;//Variables used during multiplication 
 
  	double t_perfss,t_perfse,t_perfrs,t_perfre,t_perfms,t_perfme;//Variables used for calculating the time for storing,retrieving and multiplying
     
  	cout<<"\n wait matrices are storing to database \n";
  	struct timeval start,end;
  	t=gettimeofday(&start,NULL);
  	if(t!=0)
  	cout<<"\n get time if the day failed for start of storing";  
        
        //Opening of A,B and C matrix data bases
  	LargeMatrix *adb = open_matrix(dbpath,"A2.db");
  	LargeMatrix *bdb = open_matrix(dbpath,"B2.db");
  	LargeMatrix *cdb = open_matrix(dbpath,"C2.db");

   	cout<< "\n db for A,B and C are created."<< endl;
        
        //Writing matrices to databases
   	adb->write_matrix('A',noof_a_rows,noof_a_columns,noof_rows_in_block_a,noof_columns_in_block_a);
   	bdb->write_matrix('B',noof_b_rows,noof_b_columns,noof_rows_in_block_b,noof_columns_in_block_b);
   	cdb->write_matrix('C',noof_a_rows,noof_b_columns,noof_rows_in_block_a,noof_columns_in_block_b);
   
   	t=gettimeofday(&end,NULL);
   	if(t!=0)
   	cout<<"\n get time if the day failed for end of storing";

   	t_perfss=(double)start.tv_usec/1000000;
   	t_perfss=t_perfss+(double)start.tv_sec;
   	t_perfse=(double)end.tv_usec/1000000;
   	t_perfse=t_perfse+(double)end.tv_sec;
        
        //Creating memory for matrices
   	ablockdata=(double **)malloc(noof_a_rows*sizeof(double *));
   	bblockdata=(double **)malloc(noof_b_rows*sizeof(double *));
   	cblockdata=(double **)malloc(noof_a_rows*sizeof(double *));
   
   	for(i=0;i<noof_a_rows;i++)
   	{
    		ablockdata[i]=(double *)malloc(noof_a_columns*sizeof(double));
    		cblockdata[i]=(double *)malloc(noof_b_columns*sizeof(double));
   	}
   	for(i=0;i<noof_b_rows;i++)
   	{
    		bblockdata[i]=(double *)malloc(noof_b_columns*sizeof(double));
   	}
         
   	t=gettimeofday(&start,NULL);
   	if(t!=0)
   	cout<<"\n get time if the day failed for start of retrieving";

        //Reading matrices from databases
   	adb->read_matrix(ablockdata,noof_a_rows,noof_a_columns,noof_rows_in_block_a,noof_columns_in_block_a);
  	bdb->read_matrix(bblockdata,noof_b_rows,noof_b_columns,noof_rows_in_block_b,noof_columns_in_block_b);
   	cdb->read_matrix(cblockdata,noof_a_rows,noof_b_columns,noof_rows_in_block_a,noof_columns_in_block_b);
   
   	t=gettimeofday(&end,NULL);
   	if(t!=0)
   	cout<<"\n get time if the day failed for end of retrieving";

   	t_perfrs=(double)start.tv_usec/1000000;
   	t_perfrs=t_perfrs+(double)start.tv_sec;
   	t_perfre=(double)end.tv_usec/1000000;
   	t_perfre=t_perfre+(double)end.tv_sec;
   	
        //Printing matrices 
   	cout<<"\n matrix A is \n";   
   	adb->print_matrix(ablockdata,noof_a_rows,noof_a_columns);
   	cout<<"\n matrix B is \n";
   	bdb->print_matrix(bblockdata,noof_b_rows,noof_b_columns);
   	cout<<"\n Before multiplication matrix C is \n";
   	cdb->print_matrix(cblockdata,noof_a_rows,noof_b_columns);
   	
   	struct datablock A,B,C;

        ////Creating memory for single block for matrices
   	A.blockdata=(double **)malloc(noof_rows_in_block_a * sizeof(double *) );
  	C.blockdata=(double **)malloc(noof_rows_in_block_a * sizeof(double *) );
   	for(i=0;i<noof_rows_in_block_a;i++)
   	{
    		A.blockdata[i]=(double *)malloc(noof_columns_in_block_a * sizeof(double) );
    		C.blockdata[i]=(double *)malloc(noof_columns_in_block_b * sizeof(double) );
   	} 
   	B.blockdata=(double **)malloc(noof_rows_in_block_b * sizeof(double *) );
   	for(i=0;i<noof_rows_in_block_b;i++)
   	B.blockdata[i]=(double *)malloc(noof_columns_in_block_b * sizeof(double) ); 
 
   	t=gettimeofday(&start,NULL);
   	if(t!=0)
   	cout<<"\n get time if the day failed for start of retrieving";

        //Matrix multiplication block by block
      	for(global_i=0; global_i<noof_a_row_blocks*noof_a_column_blocks; global_i++) 
      	{   
         	a_block_no++;
         	A=adb->getVal(a_block_no);
         	for(global_j=0; global_j<noof_b_column_blocks; global_j++)
         	{
             		b_block_no++;
             		c_block_no++;
             		B=bdb->getVal(b_block_no);
             		C=cdb->getVal(c_block_no);
             		for(i = 0; i <noof_rows_in_block_a; i++) 
             		{
               			for(j = 0; j <noof_columns_in_block_b; j++)
               			{
                 			double sum=C.blockdata[i][j];
                 			for(k = 0; k <noof_columns_in_block_a; k++)
                 			{
                   				sum += A.blockdata[i][k]*B.blockdata[k][j];
                 			}
                 			C.blockdata[i][j]=sum;
               			} 
             		}
           		cdb->setVal(c_block_no,C,noof_rows_in_block_a,noof_columns_in_block_b);
         	} 
         	b_block_no=0;
       	}

   	t=gettimeofday(&end,NULL);
   	if(t!=0)
   	cout<<"\n get time if the day failed for end of retrieving";

   	t_perfms=(double)start.tv_usec/1000000;
   	t_perfms=t_perfms+(double)start.tv_sec;
   	t_perfme=(double)end.tv_usec/1000000;
   	t_perfme=t_perfme+(double)end.tv_sec;
	
        //Reading and printing of resultant matrix-c
   	cdb->read_matrix(cblockdata,noof_a_rows,noof_b_columns,noof_rows_in_block_a,noof_columns_in_block_b);   
   	cout<<"\n After multiplication matrix C is \n"; 
   	cdb->print_matrix(cblockdata,noof_a_rows,noof_b_columns);
	

        //printing the time for storing,retrieving and multiplying
   	cout<<"\n Time for storing data:"<<t_perfse-t_perfss;
  
   	cout<<"\n Time for retriveing data:"<<t_perfre-t_perfrs;
 
   	cout<<"\n Time for multiplying two matrices:"<<t_perfme-t_perfms;
   	cout<<"\n";    
  
        //closing of databases
    	adb->closedb();
    	bdb->closedb();
    	cdb->closedb();
    
}

