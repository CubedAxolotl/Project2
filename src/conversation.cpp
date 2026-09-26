#include "core/conversation.h"
#include <stdexcept>


Conversation::Conversation() { //constructor
    data_ = nullptr;
    size_ = 0;
capacity_ = 0;
}

Conversation::~Conversation() { //desturctor
    delete[] data_;

}

Conversation::Conversation(const Conversation& other) {//copy constructor 
    this->size_ = other.size_;
    this->capacity_ = other.capacity_; //copies the easy 
    
    if (other.capacity_ == 0){//detects if capacity is zero
        this->data_ = nullptr;
        return;
    }

    this->data_ = new Message[other.capacity_];//allocates messages

    for (size_t i= 0; i<other.size_; i++){//copies data from other to this
        this->data_[i]=other.data_[i];
    }



}

Conversation& Conversation::operator=(const Conversation& other) {//copy assingment operator
    if(this == &other){//if pointers are pointing at the same msg
        return *this;// this = this, so return this
    }

    Message* newData = nullptr;
    if (other.capacity_ > 0) {// makes new array first
        newData = new Message[other.capacity_];

        for (size_t i = 0; i < other.size_; i++) {
            newData[i] = other.data_[i];
        }
    }

    delete[] this->data_;
    this->data_ = newData;
    this->size_ = other.size_;
    this->capacity_ = other.capacity_;

return *this;
}

Conversation::Conversation(Conversation&& other) noexcept {//move constructor
    this->size_ = other.size_;
    this->capacity_ = other.capacity_; //copies the easy stuff
    if (other.capacity_ == 0){//detects if capacity is zero
        this->data_ = nullptr;
        return;
    }

    this->data_ = other.data_;//transfers ownershit of the msg pointer

    other.size_=0; //sets other to 0 and stops other and this from sharing a pointer
    other.capacity_=0;
    other.data_ = nullptr;
}

Conversation& Conversation::operator=(Conversation&& other) noexcept {//move assignment
    if(this == &other){//if pointers are pointing at the same msg
        return *this;// this = this, so return this
    }
    this->size_ = other.size_;
    this->capacity_ = other.capacity_; //copies the easy stuff
    if (other.capacity_ == 0){//detects if capacity is zero
        delete[] this->data_;
        this->data_ = nullptr;
        return *this;
    }

    delete[] this->data_;// deletes data
    this->data_ = other.data_;//transfers ownershit of the msg pointer

    other.size_=0; //sets other to 0 and stops other and this from sharing a pointer
    other.capacity_=0;
    other.data_ = nullptr;

    return *this;
}

void Conversation::append(Message m) {
    if (capacity_==size_ && capacity_!=0){//array is full
        
        Message *newMsg = new Message[capacity_*2];


        for (size_t i= 0; i<capacity_; i++){//copies data from current msgarray to newmsgarray
            newMsg[i]=data_[i];
        }
        delete[] data_;
        
        newMsg[size_] = m;
        
        data_ = newMsg; 
        capacity_ =capacity_*2;
        size_++;

        return;

    }
    else if(capacity_>0){ //array is not empty

    data_[size_]= m;
    size_++;
    return;
    }   
    else if(capacity_ == 0){//array is empty
        Message *newMsg = new Message[1];
        capacity_=1;
        size_=1;
        newMsg[0] = m;
        data_ = newMsg;
    }
    
    return;
}

std::size_t Conversation::size() const noexcept {
    return size_;
}

const Message& Conversation::at(std::size_t i) const {
    if (i>= size_){
         throw std::out_of_range("Conversation::at: index out of range");
    }

    return data_[i];
}

const Message* Conversation::begin() const noexcept {
    return data_;
}

const Message* Conversation::end() const noexcept {
    return data_+size_;
}