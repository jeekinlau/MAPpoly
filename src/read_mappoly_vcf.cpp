/*
 MAPpoly: a package to construct genetic maps in autopolyploids
 Copyright (C) 2014-2018 Marcelo Mollinari

 This file is part of MAPpoly.

 MAPpoly is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

/*
 File: read_mappoly_vcf.cpp

 Description: Set of functions to be used with software R

 Functions used to allow .vcf input file support.
 Some of these functions were adapted from vcfR package.  

 Functions adapted by Gabriel Gesteira.

 First version:       2019
 Last update: Aug 08, 2019
 */

#include <Rcpp.h>
#include <algorithm>
#include <iostream>
#include <vector>
#include <math.h>
#include <R.h>
#include <Rmath.h>
#include <R_ext/PrtUtil.h>
#include <zlib.h>
#include <sstream>
#include <string>
using namespace std;
using namespace Rcpp;

//using namespace Rcpp;
// https://www.lemoda.net/c/gzfile-read/


// Number of records to report progress at.
const int nreport = 1000;

/* Size of the block of memory to use for reading. */
// #define LENGTH 0x1000 // hexadecimel for 4096.
// #define LENGTH 0x2000 // hexadecimel for 8192.
#define LENGTH 4000 // hexadecimel for 16384 or 16.384 KB.

//#define LENGTH 2710 // hexadecimel for 10,000 or 10 KB.
// #define LENGTH 4E20 // hexadecimel for 20,000 or 20 KB.

// Testing functions

int get_dosage(std::string mystring, int gt_pos){
  // mystring is a string containing vcf genotypes
  // gt_pos is the GT position on vcf file
  //  Rcpp::Rcout << "In strsplit" << std::endl;
  
  char split = ':';
  int flag;
  std::vector<std::string> vec_o_strings;
  int start = 0;
  unsigned int i=0;
  
  // Looping through string
  for(i = 1; i < mystring.size(); i++){
    if( mystring[i] == split){
      std::string temp = mystring.substr(start, i - start);
      vec_o_strings.push_back(temp);
      start = i+1;
      i = i+1;
    }
  }
  
  // Handle the last element
  std::string temp = mystring.substr(start, i - start);
  vec_o_strings.push_back(temp);
  
  // Testing for correct separation
  std::string test = vec_o_strings[gt_pos-1];
  if(test.size() > 2){
    mystring = vec_o_strings[gt_pos-1];
  }
  start = 0;
  
  // Checking for NA
  if(mystring[0] == '.'){
    return -1;
  }
  
  // Checking other formats
  if(mystring[0] == '0' || mystring[0] == '1'){flag = 0;} else {flag = 1;}
  if(flag == 1){
    Rcpp::Rcout << "Warning: GT field does not have the expected format, please check your file. Returning NA instead.\n";
    return -1;
  }
  
  // Accounting allele dosages
  for (i = 0; i < mystring.size(); i++){
    if(mystring[i] == '1'){
      start += 1;
    }
  }
  
  return start;
}

// Getting ploidy information
int get_ploidy(std::string mystring, int gt_pos){
  // mystring is a string containing vcf genotypes
  // gt_pos is the GT position on vcf file
  //  Rcpp::Rcout << "In strsplit" << std::endl;
  
  char split = ':';
  char split2 = '/';
  int flag;
  std::vector<std::string> vec_o_strings;
  int start = 0;
  unsigned int i=0;
  
  // Looping through string
  for(i = 1; i < mystring.size(); i++){
    if( mystring[i] == split){
      std::string temp = mystring.substr(start, i - start);
      vec_o_strings.push_back(temp);
      start = i+1;
      i = i+1;
    }
  }
  
  // Handle the last element
  std::string temp = mystring.substr(start, i - start);
  vec_o_strings.push_back(temp);
  
  // Testing for correct separation
  std::string test = vec_o_strings[gt_pos-1];
  if(test.size() > 2){
    mystring = vec_o_strings[gt_pos-1];
  }
  start = 0;
  
  // // Checking for NA (not needed for ploidy)
  // if(mystring[0] == '.'){
  //   return -1;
  // }
  
  // Checking other formats
  if(mystring[0] == '0' || mystring[0] == '1' || mystring[0] == '.'){flag = 0;} else {flag = 1;}
  if(flag == 1){
    Rcpp::Rcout << "Warning: GT field does not have the expected format, please check your file. Returning NA instead.\n";
    return -1;
  }

  // Looping through string2
  start = 0;
  for(i = 0; i < mystring.size(); i++){
    if( mystring[i] != split2){
      start += 1;
    }
  }  
  return start;
}

// Getting dosages for all markers and all individuals
//' @export
// [[Rcpp::export(name=".vcf_transform_dosage")]]
Rcpp::NumericMatrix vcf_transform_dosage(Rcpp::StringMatrix& mat, int gt_pos){
  int rowmat = mat.nrow();
  int colmat = mat.ncol();
  int k, l;
  Rcpp::NumericMatrix results(rowmat,colmat);
  
  int get_dosage(std::string mystring, int gt_pos);
  
  for (k=0; k < rowmat; k++){
    for (l=0; l < colmat; l++){
      results(k,l) = get_dosage(as<std::string>(mat(k,l)), gt_pos);
    }
  }
  return results;
}

// Getting ploidy for all markers and all individuals
//' @export
// [[Rcpp::export(name=".vcf_get_ploidy")]]
Rcpp::NumericMatrix vcf_get_ploidy(Rcpp::StringMatrix& mat, int gt_pos){
  int rowmat = mat.nrow();
  int colmat = mat.ncol();
  int k, l;
  Rcpp::NumericMatrix results(rowmat,colmat);
  
  int get_ploidy(std::string mystring, int gt_pos);
  
  for (k=0; k < rowmat; k++){
    for (l=0; l < colmat; l++){
      results(k,l) = get_ploidy(as<std::string>(mat(k,l)), gt_pos);
    }
  }
  return results;
}


// Rcpp::NumericMatrix vcf_transform_dosage(Rcpp::StringMatrix& mat, int gt_pos){
//   int rowmat = mat.nrow();
//   int colmat = mat.ncol();
//   int k, l;
//   int res;
//   char split = ':';
//   int flag;
//   std::vector<std::string> vec_o_strings;
//   int start = 0;
//   int i=0;
//   Rcpp::NumericMatrix results(rowmat,colmat);
//   std::string mystring;
//   
//   for (k=0; k < rowmat; k++){
//     for (l=0; l < colmat; l++){
//       start = 0;
//       // Defining mystring
//       mystring = mat(k,l);
//       // Rcpp::Rcout << mystring << '\n';
//       // Looping through string
//       for(i = 1; i < mystring.size(); i++){
//         if( mystring[i] == split){
//           std::string temp = mystring.substr(start, i - start);
//           vec_o_strings.push_back(temp);
//           start = i+1;
//           i = i+1;
//         }
//       }
//       
//       // Handle the last element
//       std::string temp = mystring.substr(start, i - start);
//       vec_o_strings.push_back(temp);
//       
//       // Testing for correct separation
//       std::string test = vec_o_strings[gt_pos-1];
//       if(test.size() > 2){
//         mystring = vec_o_strings[gt_pos-1];
//       }
//       start = 0;
//       
//       // Checking other formats
//       if(mystring[0] == '0' || mystring[0] == '1'){flag = 0;} else {flag = 1;}
//       
//       // Checking for NA
//       if(mystring[0] == '.'){
//         start = -1;
//       } else if(flag == 0){
//         // Accounting allele dosages
//         for (i = 0; i < mystring.size(); i++){
//           if(mystring[i] == '1'){
//             start += 1;
//           }
//         }
//       }
//       else if(flag == 1){
//         Rcpp::Rcout << "Warning: GT field does not have the expected format, please check your file. Returning NA instead.\n";
//         start = -1;
//       }
// 
//       results(k,l) = start;
//     }
//   }
//   return results;
// }

void testing_strsplit(std::string& mystring, std::vector<std::string>& vec_o_strings, char& split){
  // mystring is a string to be split on the character 'split'.
  // vec_o_strings is empty and will be pushed on to.
  //  Rcpp::Rcout << "In strsplit" << std::endl;
  
  int start = 0;
  unsigned int i=0;
  
  for(i = 1; i < mystring.size(); i++){
    if( mystring[i] == split){
      std::string temp = mystring.substr(start, i - start);
      vec_o_strings.push_back(temp);
      start = i+1;
      i = i+1;
    }
  }
  
  // Handle the last element.
  std::string temp = mystring.substr(start, i - start);
  vec_o_strings.push_back(temp);
  
}

void testing_gtsplit(std::string& mystring,
                     std::vector<std::string>& vec_o_strings,
                     int& unphased_as_na){
  
  // mystring is a string of genotypes to be split to a character.
  // Genotypes may be delimited as | or /.
  // vec_o_strings is empty and will be pushed on to.
  // Sometimes, genotypes delimited with / may be undesireable.
  // In this case missing data should be returned.
  
  //  Rcpp::Rcout << "In gtsplit\n";
  
  int start = 0;
  unsigned int i = 0;
  int is_phased = 0;
  
  char split1 = '|'; // Must be single quotes!
  char split2 = '/'; // Must be single quotes!
  
  // Iterate through genotype string looking for delimiters.  
  for(i = 0; i < mystring.size(); i++){
    if( mystring[i] == split1 ){
      // Found a delimiter.
      is_phased = 1;
      std::string temp = mystring.substr(start, i - start);
      vec_o_strings.push_back(temp);
      start = i+1;
      i = i+1;
    } else if ( mystring[i] == split2 ){
      // Found a delimiter.
      is_phased = 0;
      if( unphased_as_na == 1 ){
        vec_o_strings.push_back( "." );
      } else {
        std::string temp = mystring.substr(start, i - start);
        vec_o_strings.push_back(temp);
        start = i+1;
        i = i+1;
      }
    }
  }
  
  // Handle the last element.
  if( is_phased == 0 && unphased_as_na == 1 ){
    vec_o_strings.push_back( "." );
  } else {
    std::string temp = mystring.substr(start, i - start);
    vec_o_strings.push_back(temp);
  }
  
  // Debug
  /*
   Rcpp::Rcout << "mystring: " << mystring << " ";
   Rcpp::Rcout << "vec_o_strings: ";
   for(i=0; i<vec_o_strings.size(); i++){
   Rcpp::Rcout << " " << vec_o_strings[i];
   }
   Rcpp::Rcout << "\n";
   */
}


void testing_gtdelim(std::string& mystring,
                     std::vector<std::string>& vec_o_strings){
  // Collect a genotype's allelic delimitors.
  
  char split1 = '|'; // Must be single quotes!
  char split2 = '/'; // Must be single quotes!
  
  unsigned int i = 0;
  
  for(i = 0; i < mystring.size(); i++){
    if( ( mystring[i] == split1 ) | ( mystring[i] == split2 ) ){
      std::stringstream ss;
      ss << mystring[i];
      //      vec_o_strings.push_back( ss );
      std::string myDelim;
      ss >> myDelim;
      vec_o_strings.push_back( myDelim );
      //      vec_o_strings.push_back( split1 );
    }
  }
}

/*  Helper functions */

/* 
Called by vcf_stats_gz.
Processes lines from vcf files.
Counts meta (^##), header (^#C), columns in the header and remaining lines.
*/
void stat_line(Rcpp::NumericVector stats, std::string line){
//  Rcpp::Rcout << "    In stat_line." << std::endl;
  if(line[0] == '#' && line[1] == '#'){
    // Meta
    stats(0)++;
  } else if (line[0] == '#' && line[1] == 'C'){
    // Header
//    Rcpp::Rcout << "Found the header line.\n";
    stats(1) = stats(0) + 1;
    std::vector < std::string > col_vec;
    char col_split = '\t'; // Must be single quotes!
    testing_strsplit(line, col_vec, col_split);
    stats(3) = col_vec.size();
  } else {
    // Variant
    stats(2)++;
    // Count columns
    std::vector < std::string > col_vec;
    char col_split = '\t'; // Must be single quotes!
    testing_strsplit(line, col_vec, col_split);
    stats(4) = col_vec.size();
  }
//  Rcpp::Rcout << "    Leaving stat_line." << std::endl;
}


/*  Single pass of vcf file to get statistics */

//' @export
// [[Rcpp::export(name=".vcf_stats_gz")]]
Rcpp::NumericVector vcf_stats_gz(std::string x, int nrows = -1, int skip = 0, int verbose = 1) {
//  Rcpp::NumericVector stats(4);  // 4 elements, all zero.  Zero is default.
//  stats.names() = Rcpp::StringVector::create("meta", "header", "variants", "columns");
  Rcpp::NumericVector stats(5);  // 4 elements, all zero.  Zero is default.
  stats.names() = Rcpp::StringVector::create("meta", "header_line", "variants", "columns", "last_line");
  
  if(verbose == 1){
    Rcpp::Rcout << "Scanning file to determine attributes." << std::endl;
  }
  
  // Determine number of rows to read.
  int max_rows = 0;
  if( nrows > 0 ){
    max_rows = max_rows + nrows;
  }
  if( skip > 0 ){
    max_rows = max_rows + nrows;
  }
  
  gzFile file;
  file = gzopen (x.c_str(), "r");

  if (! file) {
    Rcpp::stop("gzopen of " + x + " failed: " + strerror (errno));
  }
//  Rcpp::Rcout << "Made it here." << std::endl;
  
  // Scroll through buffers.
  std::string lastline = "";
  while (1) {
//    Rcpp::Rcout << "Made it here." << std::endl;
    Rcpp::checkUserInterrupt();
    int err;
    int bytes_read;
    char buffer[LENGTH];
    bytes_read = gzread (file, buffer, LENGTH - 1);
    buffer[bytes_read] = '\0';
//    Rcpp::Rcout << "Buffer read in." << std::endl;

    std::string mystring(reinterpret_cast<char*>(buffer));  // Recast buffer as a string.
    mystring = lastline + mystring;
    std::vector < std::string > svec;  // Initialize vector of strings for parsed buffer.
    
    char split = '\n'; // Must be single quotes!
    testing_strsplit(mystring, svec, split);

    // Scroll through lines derived from the buffer.
    unsigned int i = 0;
    for(i=0; i < svec.size() - 1; i++){
      stat_line(stats, svec[i]);
    }
      
    // If we've specified a maximum number of rows and we've hit it,
    // we need o bail out.
    if( max_rows > 0 && stats(2) > max_rows ){
      gzclose (file);
      stats(2) = max_rows;
      return stats;
    }
    
    // Manage the last line.
    lastline = svec[svec.size() - 1];
//    Rcpp::Rcout << "  Last line managed." << std::endl;
    
    // Check for EOF or errors.
//    Rcpp::Rcout << "  Check for errors." << std::endl;
    if (bytes_read < LENGTH - 1) {
      if ( gzeof (file) ) {
//        Rcpp::Rcout << "    Found EOF." << std::endl;
//        lastline = svec[svec.size() - 1];
        break;
      }
      else {
//        Rcpp::Rcout << "    Found error_string." << std::endl;
        const char * error_string;
        error_string = gzerror (file, & err);
        if (err) {
          Rcpp::stop(std::string("Error: ") + error_string + ".");
        }
      }
    }
//    Rcpp::Rcout << "  End check for errors." << std::endl;
  }
  gzclose (file);

//  Rcpp::Rcout << "Made it to the end of vcf_stats_gz" << std::endl;
//  stats(4) = stats(1) + stats(2);
  return stats;
}


/*  Read vcf meta region  */

//' @export
// [[Rcpp::export(name=".read_meta_gz")]]
Rcpp::StringVector read_meta_gz(std::string x, Rcpp::NumericVector stats, int verbose) {
  // Read in the meta lines.
  // stats consists of elements ("meta", "header", "variants", "columns");

  Rcpp::StringVector meta(stats[0]);
  std::string line;  // String for reading file into
  int meta_row = 0;

  gzFile file;
  file = gzopen (x.c_str(), "r");
  if (! file) {
    Rcpp::stop("gzopen of " + x + " failed: " + strerror (errno) + ".");
  }

  // Scroll through buffers.
  std::string lastline = "";
  while (1) {
    Rcpp::checkUserInterrupt();
    int err;
    int bytes_read;
    char buffer[LENGTH];
    bytes_read = gzread (file, buffer, LENGTH - 1);
    buffer[bytes_read] = '\0';

    std::string mystring(reinterpret_cast<char*>(buffer));  // Recast buffer as a string.
    mystring = lastline + mystring;
    std::vector < std::string > svec;  // Initialize vector of strings for parsed buffer.
    char split = '\n'; // Must be single quotes!
    testing_strsplit(mystring, svec, split);


    unsigned int i = 0;
    while(meta_row < stats(0) && i < svec.size() - 1){
      
      // Check and remove carriage returns (Windows).
      if( svec[i][ svec[i].size()-1] == '\r' ){
        svec[i].erase( svec[i].size() - 1 );
      }

      meta(meta_row) = svec[i];
      meta_row++;
      i++;
      if(meta_row % nreport == 0 && verbose == 1){
        Rcpp::Rcout << "\rMeta line " << meta_row << " read in.";
      }
    }
    lastline = svec[svec.size() - 1];


    // Check for EOF or errors.
    if (bytes_read < LENGTH - 1) {
      if (gzeof (file)) {
        break;
      }
      else {
        const char * error_string;
        error_string = gzerror (file, & err);
        if (err) {
          Rcpp::stop(std::string("Error: ") + error_string + ".");
        }
      }
    }
  }
  gzclose (file);

  if(verbose == 1){
    Rcpp::Rcout << "\rMeta line " << meta_row << " read in.";
    Rcpp::Rcout << "\nAll meta lines processed.\n";
  }

  return meta;
}



/*  Read in the body of the vcf file  */

/*  
Helper function to process one line.
gt is a pointer to the data matrix output by read_vcf.
var_num
myline is a tab delimited string row from the orriginal vcf file.
cols is a vector indicating which columns to select from the file data.
*/
void proc_body_line(Rcpp::CharacterMatrix gt,
                    int var_num,
                    std::string myline,
                    Rcpp::IntegerVector cols,
                    int convertNA){
  
  char split = '\t'; // Must be single quotes!
  std::vector < std::string > data_vec;
  
  testing_strsplit(myline, data_vec, split);

  for(int i = 0; i < cols.size(); i++){
    if( convertNA == 1 ){
      // The VCF specification encodes missing data as ".".
      // Missing genotypes may be encoded as "./.", ".|.", etc.
      // Here we convert is to NA.
      if( data_vec[ cols[i] ] == "." ){
        gt(var_num, i) = NA_STRING;
      } else {
        // Possible genotype missing.
        std::vector < std::string > allele_vec;
        int unphased_as_na = 0; // 0 == FALSE
        std::string my_string;
        my_string = data_vec[ cols[i] ];

        
        testing_gtsplit( my_string, allele_vec, unphased_as_na );
        int gtNA = 1;
        unsigned int k = 0;
        for( k = 0; k < allele_vec.size(); k++ ){
//            Rcpp::Rcout << "allele_vec[k]: " << allele_vec[k] << "\n";
          if( allele_vec[k] != "." ){ gtNA = 0; }
        }
        if( gtNA == 1 ){
          gt(var_num, i) = NA_STRING;
        } else {
          gt(var_num, i) = data_vec[ cols[i] ];
        }
      }

    } else {
      gt(var_num, i) = data_vec[ cols[i] ];
    }
  }
}

/* 
 Read in the fixed and genotype portion of the file.
 x is the file name.
 Stats is a vector containing information about the file contents (see below).
 varbose indicates whether verbose output should be generated.
 
 Stats contains:
 "meta", "header", "variants", "columns"
 
*/

//' @export
// [[Rcpp::export(name=".read_body_gz")]]
Rcpp::CharacterMatrix read_body_gz(std::string x,
                                   Rcpp::NumericVector stats,
                                   long int nrows = -1,
                                   long int skip = 0,
                                   Rcpp::IntegerVector cols = 0,
                                   int convertNA = 1,
                                   int verbose = 1) {

  // NA matrix for unexpected results.
  Rcpp::StringMatrix na_matrix(1,1);
  na_matrix(0,0) = NA_STRING;
  
  // if(verbose == 1){
  //   Rcpp::Rcout << "In function read_body_gz." << std::endl;
  //   Rcpp::Rcout << "  stats(0): " << stats(0) << std::endl;
  //   Rcpp::Rcout << "  stats(1): " << stats(1) << std::endl;
  //   Rcpp::Rcout << "  stats(2): " << stats(2) << std::endl;
  //   Rcpp::Rcout << "  stats(3): " << stats(3) << std::endl;
  // }
  
  /*
   * Manage cols vector.
   * The first eight (1-based) columns are mandatory.
   * We can ensure they are there by adding them,
   * sorting and removing adjacent non-identical values.
   */
  for( int i=8; i >= 1; i-- ){
    cols.push_front(i);
  }
  cols.sort();

  // Remove duplicate values using a set.
  std::set<int> s( cols.begin(), cols.end() );
  cols.assign( s.begin(), s.end() );

  cols = cols - 1; // R is 1-based, C is 0-based.

  
  // Initialize matrix for body data.
  long int row_num = 0;
  

  if( ( nrows == -1 ) & ( skip == 0 ) ){
    nrows = stats[2];
  } else if ( ( nrows != -1 ) & ( skip == 0 ) ){
    // nrows = nrows;
  } else if ( ( nrows == -1 ) & ( skip > 0) ){
    nrows = stats[2] - skip;
  } else if ( ( nrows != -1 ) & ( skip > 0) ){
    // nrows = nrows;
  } else {
    Rcpp::stop("Failed to calculate return matrix geometry.");
  }
  

  // if(verbose == 1){
  //   Rcpp::Rcout << "Initializing gt matrix." << std::endl;
  //   Rcpp::Rcout << "  nrows: " << nrows << std::endl;
  //   Rcpp::Rcout << "  cols.size(): " << cols.size() << std::endl;
  // }
  
  if( nrows > INT_MAX ){
    std::stringstream ss;
    ss << "Requested a matrix of " << nrows << " rows." << std::endl;
    ss << "This exceeds INT_MAX, which is " << INT_MAX << "." << std::endl;
    ss << "I suggest you attempt to read in a portion of the file using the options 'nrows' and 'skip'." << std::endl;
    Rcpp::stop(ss.str());
  }
  
  Rcpp::CharacterMatrix gt( nrows, cols.size() );

  if(verbose == 1){
    Rcpp::Rcout << "gt matrix initialized." << std::endl;
  }
  
  row_num = 0;
  
  if( verbose == 1 ){
    Rcpp::Rcout << "Character matrix gt created.\n";
    Rcpp::Rcout << "  Character matrix gt rows: ";  Rcpp::Rcout << gt.rows();
    Rcpp::Rcout << "\n";
    Rcpp::Rcout << "  Character matrix gt cols: ";  Rcpp::Rcout << gt.cols();
    Rcpp::Rcout << "\n";
    Rcpp::Rcout << "  skip: ";  Rcpp::Rcout << skip;
    Rcpp::Rcout << "\n";
    Rcpp::Rcout << "  nrows: ";  Rcpp::Rcout << nrows;
    Rcpp::Rcout << "\n";
    Rcpp::Rcout << "  row_num: ";  Rcpp::Rcout << row_num;
    Rcpp::Rcout << "\n";
//    Rcpp::Rcout << "\n";
  }

  
  // Create filehandle and open.
  gzFile file;
  file = gzopen (x.c_str(), "r");
  if (! file) {
    Rcpp::stop("gzopen of " + x + " failed: " + strerror (errno) + ".");
    return na_matrix;
  }


  // Because the last line may be incomplete,
  // We'll typically omit it from processing and
  // concatenate it to the first line.
  // But first we'll have to initialize it.
  std::string lastline = "";
  
  // String vector to store the header (^#CHROM...).
  std::vector<std::string> header_vec;
  
  // variant counter.  
  long int var_num = 0;


  // Scroll through buffers.
  while (1) {
    Rcpp::checkUserInterrupt();
    int err;

    // Slurp in a buffer.
    int bytes_read;
    char buffer[LENGTH];
    bytes_read = gzread (file, buffer, LENGTH - 1);
    buffer[bytes_read] = '\0'; // Terminate the buffer.

    std::string mystring(reinterpret_cast<char*>(buffer));  // Recast buffer as a string.
    mystring = lastline + mystring; // Concatenate last line to the buffer
    
    // Delimit into lines.
    std::vector < std::string > svec;  // Initialize vector of strings for parsed buffer.
    char split = '\n'; // Must be single quotes!
    testing_strsplit(mystring, svec, split);
    
    /* 
    svec should now contain a vector of strings,
    one string for each line
    where the last line may be incomplete.
    We can now process each line except the last.
    */

    // Scroll through lines.
    unsigned int i = 0;
    for(i = 0; i < svec.size() - 1; i++){
      
      // Check and remove carriage returns (Windows).
      if( svec[i][ svec[i].size()-1] == '\r' ){
        svec[i].erase( svec[i].size() - 1 );
      }

      if(svec[i][0] == '#' && svec[i][1] == '#'){
        // Meta line, ignore.
      } else if(svec[i][0] == '#' && svec[i][1] == 'C'){
        // Process header.
//        Rcpp::Rcout << svec[i].substr(0,40) << "\n\n";
        char header_split = '\t';
        testing_strsplit(svec[i], header_vec, header_split);
        
        // Subset the header to select columns.
        std::vector<std::string> header_vec2( cols.size() );
        for(int j=0; j<cols.size(); j++){
          header_vec2[j] = header_vec[ cols[j] ];
        }
        header_vec = header_vec2;
      } else {
        // Variant line.

        if ( ( var_num >= skip ) & ( row_num < nrows ) ){
          proc_body_line(gt, row_num, svec[i], cols, convertNA);
          row_num++; // Return matrix row number.
        }
        var_num++; // Input row number.


        if(var_num % nreport == 0 && verbose == 1){
          Rcpp::Rcout << "\rProcessed variant " << var_num;
        }
      }   
    }

    
    // Processed all lines of current buffer.
    // Keep the last line so we can append it to 
    //the beginning of the next buffer.
    lastline = svec[svec.size() - 1];

//    Rcpp::Rcout << "line-2:" << svec[svec.size() - 2].substr(0,40) << "|<-\n";
//    Rcpp::Rcout << "line-1:" << svec[svec.size() - 1].substr(0,40) << "|<-\n";
//    Rcpp::Rcout << "\n";

      
    /*
     * We can bail out early if we have read nrows.
     * Before we do we need to check that:
     * 1) we have read in nrows
     * 2) we have processed the header
     * (important when nrows is small)
     * 3) we actually have a line (when buffer ends at the end of a line).
     */
    if( ( row_num >= nrows ) & ( lastline[0] != '#' ) & ( lastline.size() > 0 ) ){
      break;
    }


    // Check for EOF or errors.
    if (bytes_read < LENGTH - 1) {
      if (gzeof (file)) {
        break;
      }
      else {
        const char * error_string;
        error_string = gzerror (file, & err);
        if (err) {
          Rcpp::stop(std::string("Error: ") + error_string + ".");
        }
      }
    }
    
  // Return to top of loop and process another buffer.
  } // Close while.
  
  // Close filehandle.
  gzclose (file);
  
  if( stats[1] == 0 ){
    if( verbose == 1 ){
      Rcpp::Rcout << "Warning: no header information was found! Data contains no sample names!\n";
    }
  } else {

    if( header_vec.size() == (unsigned)gt.ncol() ){
      header_vec[0] = "CHROM";
      gt.attr("dimnames") = Rcpp::List::create(Rcpp::CharacterVector::create(), header_vec);
    } else {
      if( verbose == 1 ){
        Rcpp::Rcout << "Warning: no header information found!\n";
      }
    }
  }

//  Rcpp::Rcout << "\n\n>>---<< Made it! >>---<<\n\n";

  if(verbose == 1){
    Rcpp::Rcout << "\rProcessed variant: " << var_num;
    Rcpp::Rcout << "\nAll variants processed\n";
  }

  return gt;
}
