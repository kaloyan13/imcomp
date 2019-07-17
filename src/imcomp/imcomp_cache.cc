#include "imcomp/imcomp_cache.h"


Node* DoublyLinkedList::add_page_to_head(string key, vector<VlSiftKeypoint> &features, vector< vector<vl_uint8> > &descriptors) {
  Node *page = new Node(key, features, descriptors);
  if(!front && !rear) {
    front = rear = page;
  }
  else {
    page->next = front;
    front->prev = page;
    front = page;
  }
  return page;
}

void DoublyLinkedList::move_page_to_head(Node *page) {
  if(page==front) {
    return;
  }
  if(page == rear) {
    rear = rear->prev;
    rear->next = NULL;
  }
  else {
    page->prev->next = page->next;
    page->next->prev = page->prev;
  }

  page->next = front;
  page->prev = NULL;
  front->prev = page;
  front = page;
}

void DoublyLinkedList::remove_rear_page() {
  if(is_empty()) {
    return;
  }
  if(front == rear) {
    delete rear;
    front = rear = NULL;
  }
  else {
    Node *temp = rear;
    rear = rear->prev;
    rear->next = NULL;
    delete temp;
  }
}

Node* DoublyLinkedList::get_rear_page() {
  return rear;
}


bool imcomp_cache::get(string key, vector<VlSiftKeypoint> &features,  vector< vector<vl_uint8> > &descriptors) {
  if(pageMap.find(key) == pageMap.end()) {
    // not found in the cache
    return false;
  }

  features = pageMap[key]->features;
  descriptors = pageMap[key]->descriptors;

  pageList->move_page_to_head(pageMap[key]);
  return true;
}

void imcomp_cache::put(string key, vector<VlSiftKeypoint> &features, vector< vector<vl_uint8> > &descriptors) {
  if(pageMap.find(key)!=pageMap.end()) {
    // if key already present, update value and move page to head
    pageMap[key]->features = features;
    pageMap[key]->descriptors = descriptors;
    pageList->move_page_to_head(pageMap[key]);
    return;
  }

  if(size == capacity) {
    // remove rear page
    string k = pageList->get_rear_page()->key;
    pageMap.erase(k);
    pageList->remove_rear_page();
    size--;
  }

  // add new page to head to Queue
  Node *page = pageList->add_page_to_head(key, features, descriptors);
  size++;
  pageMap[key] = page;
}
