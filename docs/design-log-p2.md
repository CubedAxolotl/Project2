# Design Log — Project 2

(500–800 words total. See spec §5 for what each section must cover.)

## Growth factor and amortized cost
As a conversation objects  tracks data on the heap it also keeps track of its capacity. When appending to already existing data it compares these two values and if data is full it grows the capacity of the data to twice its original size, if capacity is zero it grows to 1. So for example;If we keep appending from zero to 64 values, the data will grow in capacity in the following rate: 0->1->2->4->8->16->32->64 which is O(1). 


## Rule of Five evidence
Destructor: Destructor deletes data_

Copy Constructor: The copy constructor has safeguards to detect if the data your copying is empty, if it is, it sets data to nullptr. Otherwise it copies the data into the a new message array

Copy Assignment operator: The copy assingment operator checks if the two string in the operation are the the same. then it checks if the capacity of the message thats being copied from is 0, if so it deletes the data of the message its copying to and sets its data pointer to nullptr. Else it copies the data into a new message and makes the copied message data pointer to point to said copied data.

Move Constructor: The move constructor has safeguards to detect if the data your copying is empty, if it is, it sets data to nullptr. Otherwise it moves the pointers so the message you are copying to points to the data from the message you are copying from. then it makes the message youre copying from to equal zero along with its data pointer to null so they don't share ownership of the data.

Move assingment operator:The copy assingment operator checks if the two string in the operation are the the same. then it checks if the capacity of the message thats being copied from is 0, if so it deletes the data of the message its copying to and sets its data pointer to nullptr. Else it moves the pointers so the message you are copying to points to the data from the message you are copying from. then it makes the message youre copying from to equal zero along with its data pointer to null so they don't share ownership of the data.

## Sentinel scanner: bounded pending_ proof
The sentinel scanner keeps a string called pending that holds the end of the previous chunk, in case it is the start of the sentinel. Each time feed is called, it joins pending with the new chunk and searches for the sentinel. If it finds it, everything before the sentinel is printed and pending is cleared. If not, everything except the last m − 1 characters is printed
and those last characters become the new pending. if there are fewer than m − 1 characters, all of them are kept. When the reply ends, flush() prints whatever is left and clears it. So pending can never hold more than 19 characters.

## What I would change differently
This project is extrememly complex for it to be our second project, there a some pieces of code that we were given that I still fully understand. The code is just so dense. The spec can be vague at times and I found it confusing.