/** @file   imcomp_cache.h
 *  @brief  Enables application level caching
 *
 *
 *  @author Shrinivasan Sankar (sankar@robots.ox.ac.uk)
 *  @date   16 July 2019
 */

 #ifndef _IMCOMP_CACHE_H_
 #define _IMCOMP_CACHE_H_

extern "C" {
 #include <vl/generic.h>
 #include <vl/stringop.h>
 #include <vl/pgm.h>
 #include <vl/sift.h>
 #include <vl/getopt_long.h>
}

 #include <iostream>
 #include <map>
 #include <vector>

 using namespace std;

 class Node {
 	public:
   	string key;
    // the node has both key points and its descriptors
    vector<VlSiftKeypoint> features;
    vector< vector<vl_uint8> > descriptors;
   	Node *prev, *next;
   	Node(string k, vector<VlSiftKeypoint> f, vector< vector<vl_uint8> > d): key(k), features(f), descriptors(d), prev(NULL), next(NULL) { }
 };


class DoublyLinkedList {
  Node *front, *rear;

  bool is_empty() {
    return rear == NULL;
  }

  public:
    DoublyLinkedList(): front(NULL), rear(NULL) { }
    Node* add_page_to_head(string key, vector<VlSiftKeypoint> &features, vector< vector<vl_uint8> > &descriptors);
    void move_page_to_head(Node *page);
    void remove_rear_page();
    Node* get_rear_page();
};


class imcomp_cache {
  int capacity, size;
  DoublyLinkedList *pageList;
  map<string, Node*> pageMap;

  public:
    imcomp_cache(int k) {
      this->capacity = capacity;
    	size = 0;
      pageList = new DoublyLinkedList();
      pageMap = map<string, Node*>();
    }


    bool get(string key, vector<VlSiftKeypoint> &features, vector< vector<vl_uint8> > &descriptors);

    void put(string key, vector<VlSiftKeypoint> &features, vector< vector<vl_uint8> > &descriptors);

    ~imcomp_cache() {
    	map<string, Node*>::iterator i1;
    	for(i1 = pageMap.begin(); i1!=pageMap.end(); i1++) {
    		delete i1->second;
    	}
    	delete pageList;
    }

};

#endif
