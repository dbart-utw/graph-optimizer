// Binary Heap implementation used for dijkstra's algorithm.
// Extracted from https://github.com/gkhnyllmz/Betweenness-Centrality/blob/main/old/BinaryHeap.h

#include <iostream>
#include <vector>
#include <climits>

typedef struct{
    int node;
    int key;
} bnode;

class BinaryHeap{

    private:

        int size = 0;
        std::vector<bnode> heap;
        std::vector<int> pos; // Will be used for decrase_key

    public:

        // Creates a binary heap with size 0
        BinaryHeap(){}

        // Creates a binary heap with size times item with INF key
        BinaryHeap(int size){
            for(int i=0; i<size; i++){
                this->insert(i,INT_MAX);
            }
        }

        // Returns parent of an item
        int get_parent(int index){
            return (index-1)/2;
        }

        // Returns left child of an item
        int get_left(int index){
            return 2*index+1;
        }

        // Returns right child of an item
        int get_right(int index){
            return 2*index+2;
        }

        // Returns true if heap is empty, false otherwise
        bool is_empty(){
            return heap.empty();
        }
    
        int get_size() {
            return size;
        }

        // Inserts an item with node number and key end of the array, and rearranges the heap
        void insert(int node, int key){
            heap.push_back(bnode{node, key});
            pos.push_back(size);
            int index = size;
            int parent = get_parent(index);
            while(index >= 0 && heap[parent].key > key){ // If it is smaller than its parent, swap them
                swap(parent, index);
                index = parent;
                parent = get_parent(index);
            }
            size++;
        }

        // Returns the node with minimum key, and rearranges the heap
        int extract_min(){
            int min = heap[0].node;
            swap(0, size-1); // Swap with node which is at end of the heap
            heap.erase(heap.begin()+size-1); // Erase the min from end
            size--;
            reorganize(0);
            pos[min] = -1; // Set position of node when it is deleted
            return min;
        }

        // Sets the key value of node to n_val, an rearranges the heap
        void decrease_key(int node, int n_val){
            int index = pos[node]; // Find index of given node
            heap[index].key = n_val;
            int parent = get_parent(index);
            while(index >= 0 && heap[parent].key > n_val){ // Swap with parent if it is smaller than its parent
                swap(parent, index);
                index = parent;
                parent = get_parent(index);
            }
        }

        // Rearranges the heap starting from index to end
        void reorganize(int index){
            int smallest, left, right; 
            smallest = index; 
            left = 2*index + 1; 
            right = 2*index + 2; 

            if(left < size){ // If node has no child do nothing
                if(right >= size && heap[left].key < heap[index].key){ // If there is no right child, only left can be the smallest
                    smallest = left;
                }

                else if (heap[left].key < heap[index].key &&  heap[left].key <= heap[right].key){
                    smallest = left; 
                }
                    
                else if (heap[right].key < heap[index].key &&  heap[right].key <= heap[left].key){
                    smallest = right; 
                }
            
                if (smallest != index && smallest < size){
                    swap(smallest, index); 
                    reorganize(smallest); 
                }
            } 
        }

        // Swaps with item in n1 and item in n2, swaps positions also
        void swap(int i1, int i2){
            bnode temp = heap[i1];
            pos[heap[i1].node] = i2; // Swap positions of nodes
            pos[heap[i2].node] = i1;
            heap[i1] = heap[i2];
            heap[i2] = temp;  
        }

        // Prints the heap
        // Ex: a:b c:d e:f means a is the first node in the heap with key b, c is the left child of a with key d, e is the right child of a with key f
        void print(){
            int size = heap.size();
            for(int i=0; i<size; i++){
                std::cout << heap[i].node << ":" << heap[i].key << " ";
            }
            std::cout << std::endl;
        }

        // Prints the positions of nodes in the heap
        // a:b means node a is at index b in the heap
        void print_pos(){
            for(uint i=0; i<pos.size(); i++){
                std::cout << i << ":" << pos[i] << " ";
            }
            std::cout << std::endl;
        }
};