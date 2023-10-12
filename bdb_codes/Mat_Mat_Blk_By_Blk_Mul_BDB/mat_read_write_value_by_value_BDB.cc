#include <db_cxx.h>
#include <vector>
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
class BerkeleyDB
{
 public:
  BerkeleyDB(const string &path, const string &dbName,
	     u_int32_t cflags = 0);

  ~BerkeleyDB() { close(); }

    template<class T>
    int put( size_t &k, const T &d)
    {
      size_t dsize = sizeof(T);
      Dbt key((void*)&k, sizeof(size_t) );
      Dbt dbtdata( (void*)&d, dsize);
      int stat =   db.put(0, &key, &dbtdata, 0);
      return stat;
    }

    template <class T>
    int get( size_t &k, T &d)
    {
      Dbt key((void*)&k, sizeof(size_t) );
      Dbt dbtdata;
      int stat = db.get( 0, &key, &dbtdata, 0);
      size_t  dsize = sizeof(T);
      memcpy( &d, dbtdata.get_data(), dsize);
      return stat;
    }
 private:
  Db db;
  string dbFileName;
  BerkeleyDB() : db(0, 0) {}
  void close();
};

inline BerkeleyDB::BerkeleyDB(const string &path, const string &dbName,u_int32_t cflags) : db(0, 0), dbFileName(path + dbName)
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

inline void BerkeleyDB::close()
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
 public:
  LargeMatrix() {  }
  LargeMatrix(const string &pname, const string &dname) { pathname = pname; dbname = dname; }

  void setPath( const string &s) { pathname = s;}
  void setName( const string &s) { dbname = s;}

  void openbdb( const int mode = DB_CREATE) const 
  {
       db = new BerkeleyDB(pathname, dbname, mode);
  }
  void closedb()
  {
       if( db ) delete db; db = 0;
  }

  void    setDimension( int a,int b)  
  {
    n=a;
    m=b; 
  }
  void    setVal(int c, int &val)
          {
              size_t index =c;
              db->put(index, val);
          }

int  getVal(int i,int j)
          {
              size_t index = getIndex(i,j);
              int buf;
              db->get(index, buf);
              return buf;
          }
int  getIndex(int i, int j )
          {
            assert( i >=0 && i <n );
            assert( j >=0 && j <m );
            return (i*n)+(j*m);
          }
 private:
  mutable BerkeleyDB  *db;
  size_t  n,m;
  string    pathname, dbname;
};


LargeMatrix *open_matrix( const string &dbpathname,const string &dbname, size_t n,size_t m)
{
  LargeMatrix *adb = new LargeMatrix(dbpathname, dbname);
  adb->setDimension(n,m);
  adb->openbdb(DB_CREATE);
  return adb;
}

int main(int argc, char **argv)
{
  //assert( argc == 4 );

  if( argc != 4)
        {
                cout<<"USAGE: ./[OBJECT-FILE] [MATRIX-A-ROWS] ][MATRIX-A-COLUMNS] [DBPATH] \n";
                exit(-1);
        }
      

  size_t  n= atoi( argv[1] );
  size_t  m= atoi( argv[2] );
  string  dbpath =  argv[3];
  int block;
  cout <<" matsize :" <<n<<"*"<<m<<endl;
  cout <<" dbpath :" << dbpath << endl;

  int i,j,k,c,t;
  double t_perfss,t_perfse,t_perfrs,t_perfre;

  cout<<"\n wait matrix is storing to database";
  struct timeval start,end;
  t=gettimeofday(&start,NULL);
  if(t!=0)
  cout<<"\n get time if the day failed for start of storing";   
    
  LargeMatrix *adb = open_matrix(dbpath,"A.db",n,m);
  cout<< "\n db for A  created."<< endl;
  
   for(i = 0; i <n; i++) 
   {
     for(j = 0; j <m; j++) 
     {
       block = i+j;
       c=adb->getIndex(i,j);
       adb->setVal(c,block);
     }
   } 
   
   t=gettimeofday(&end,NULL);
   if(t!=0)
   cout<<"\n get time if the day failed for end of storing";
   t_perfss=(double)start.tv_usec/1000000;
   t_perfss=t_perfss+(double)start.tv_sec;
   t_perfse=(double)end.tv_usec/1000000;
   t_perfse=t_perfse+(double)end.tv_sec;   

   cout<<"\n";
   cout<<"The matrix is"<<"\n \n";
   
   t=gettimeofday(&start,NULL);
   if(t!=0)
   cout<<"\n get time if the day failed for start of retrieving";
   
   for(i=0; i<n; i++) 
   { 
    for(j=0; j<m;j++) 
    {
      cout<< adb->getVal(i,j)<<" ";
    }
    cout<<"\n";
   }

   t=gettimeofday(&end,NULL);
   if(t!=0)
   cout<<"\n get time if the day failed for end of retrieving";
   t_perfrs=(double)start.tv_usec/1000000;
   t_perfrs=t_perfrs+(double)start.tv_sec;
   t_perfre=(double)end.tv_usec/1000000;
   t_perfre=t_perfre+(double)end.tv_sec;
   
   cout<<"\n Time for storing data:"<<t_perfse-t_perfss;
   cout<<"\n Time for retriveing data:"<<t_perfre-t_perfrs; 
   adb->closedb();
 }

