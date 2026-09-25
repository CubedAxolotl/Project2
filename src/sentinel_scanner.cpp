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

    size_t combinedIndex = combined.find(sentinel_);// index used to delete the sentinel_ text

    if (combined.find(sentinel_)!=std::string::npos){//checks if sentinel is found between strings
    size_t combinedIndex = combined.find(sentinel_);// index used to delete the sentinel_ text
    output.sentinel_found= true;
    output.safe_text = combined.substr(0, combinedIndex);
    pending_ = "";
    return output;
   }else { //if no sentinel is found
    size_t keepSentinelSize = sentinel_.size()-1;

    if(keepSentinelSize>combined.size()){
        keepSentinelSize=combined.size();
    }
    size_t keepSafeSize = combined.size() - keepSentinelSize;

   

    std::string safeText = combined.substr(0,keepSafeSize);

    pending_= combined.substr(keepSafeSize);

    output.sentinel_found = false;
    output.safe_text = safeText;
    return output;
   }
}
 
SentinelScanner::Out SentinelScanner::flush() {
}