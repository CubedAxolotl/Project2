#include "sentinel_scanner.h"


 
SentinelScanner::SentinelScanner(std::string sentinel) { //constructor
    sentinel_ = sentinel;
    pending_ = "";
}
 
SentinelScanner::Out SentinelScanner::feed(std::string_view chunk) {
    

    std::string combined = pending_;
    combined.append(chunk);//combines last and current chunk for comparison

    std::string chunkcopy; //makes a editable copy of chunk
    chunkcopy.append(chunk);


    Out output = {"", false};//declares out

    size_t index = combined.find(sentinel_);// index used to delete the sentinel_ text


   if (combined.find(sentinel_)!=std::string::npos){

    output.sentinel_found= true;



    output.safe_text = combined.substr(0, index);
    pending_ = "";
    return output;
   }
   size_t keepSize = sentinel_.size()-1;

   if (combined.size()<= keepSize){
    output.safe_text = "";

    pending_ = combined;
   }
   else {

   output.safe_text = combined.substr(0,combined.size()-keepSize);

   pending_ = combined.substr(combined.size()-keepSize);
   }
    return output;
}
 
SentinelScanner::Out SentinelScanner::flush() {
}