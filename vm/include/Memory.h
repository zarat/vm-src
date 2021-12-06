/*
Some structures to represent the vm's memory
@todo
*/

struct node {
   int key;
   int len;
   unsigned char *data;
   struct node *next;
};

struct node *head = NULL;
struct node *current = NULL;

int memLen() {
   int length = 0;
   struct node *current;	
   for(current = head; current != NULL; current = current->next) {
      length++;
   }	
   return length;
}

void sort() {
   int i, j, k, tempKey, tempLen;
   unsigned char *tempData;
   struct node *current;
   struct node *next;	
   int size = memLen();
   k = size ;	
   for ( i = 0 ; i < size - 1 ; i++, k-- ) {
      current = head;
      next = head->next;		
      for ( j = 1 ; j < k ; j++ ) {   
         if ( current->key > next->key ) {
            tempData = current->data;
            current->data = next->data;
            next->data = tempData;
            tempKey = current->key;
            current->key = next->key;
            next->key = tempKey;
            tempLen = current->len;
            current->len = next->len;
            next->len = tempLen;
         }			
         current = current->next;
         next = next->next;
      }
   }   
}

int insertFirst(int tkey, unsigned char *tdata, int tlen) {
   struct node *link = (struct node*) malloc(sizeof(struct node));	
   if(link != NULL) {
       link->key = tkey;
       link->data = tdata;
       link->len = tlen;
       link->next = head;
       head = link;
   } else printf("[hash] could not allocate memory!\n");
   sort();
   return link->key;
}

struct node* deleteFirst() {
   struct node *tempLink = head; 
   head = head->next;
   return tempLink;
}

struct node* find(int key) {
   struct node* current = head;
   if(head == NULL) {
      return NULL;
   }
   while(current->key != key) {
      if(current->next == NULL) {
         return NULL;
      } else {
         current = current->next;
      }
   }      
   return current;
}

struct node* deleteNode(int key) {
   struct node* current = head;
   struct node* previous = NULL;
   if(head == NULL) {
      return NULL;
   }
   while(current->key != key) {
      if(current->next == NULL) {
         return NULL;
      } else {
         previous = current;
         current = current->next;
      }
   }
   if(current == head) {
      head = head->next;
   } else {
      previous->next = current->next;
   }    	
   return current;
}

