typedef struct {
	int con_sock;
    int id;
    int elo;
    bool lobby;
    int other_sock;
} user;

typedef struct listNode {
	user ListUser;
	struct listNode *next;
	struct listNode *prev;
}List;

//------ DANH SACH CAC THAO TAC TREN LIST ------

user createUser(int con_sock);
List* createClient(user listUser);
void addClient(List **list, user listUser);
List* findClient(List *list, int con_sock);
void removeClient(List **list, int con_sock);
void updateClient(List *list, int con_sock, int elo);
void updateIdClient(List *list, int con_sock, int id);
void updateOtherClient(List *list, int con_sock, int other_sock);
void addLobby(List *list, int con_sock);
void removeLobby(List *list, int con_sock);
int getSize(List *list);
int getSizeLobby(List *list);
 
// ----------------------------------------------

user createUser(int con_sock) {
    user newUser;
    newUser.con_sock = con_sock;
    newUser.id = 0;
    newUser.elo = -1;
    newUser.lobby = false;
    newUser.other_sock = -1;
    return newUser;
}

List *createClient (user listUser){
	List* newList = (List*)malloc(sizeof(List));
	newList->ListUser.con_sock = listUser.con_sock;
    newList->ListUser.elo = listUser.elo;
    newList->ListUser.id = listUser.id;
    newList->ListUser.lobby = listUser.lobby;
    newList->ListUser.other_sock = listUser.other_sock;
	newList->next = NULL;
	newList->prev = NULL;
	return newList;
}

void addClient (List **list, user listUser){
	List* newList = createClient(listUser);
	if((*list)==NULL) *list = newList;
	else{
		List *p = *list;
		while(p->next!=NULL) p = p->next;
		p->next = newList;
		newList->prev = p;
	}
}

List* findClient(List *list, int con_sock){
	while(list!=NULL){
		if(list->ListUser.con_sock == con_sock){
			return list;
		}
		list = list->next;
	}
	return NULL;
}


void removeClient(List **list,int con_sock){
	if((*list)==NULL) return;
	List *p;
	if((*list)->ListUser.con_sock == con_sock){
		p = *list;
		*list = (*list)->next;
		if((*list)!=NULL) (*list)->prev = NULL;
		free(p);
	}else{
		p = *list;
		while(p->next!=NULL){
			if(p->ListUser.con_sock == con_sock) break;
			p = p->next;
		}
		if(p==NULL) return;
		p->prev->next = p->next;
		free(p);
	}
}

void updateClient(List *list, int con_sock, int elo){
    List *p = findClient(list, con_sock);
    p->ListUser.elo = elo;
}

void updateIdClient(List *list, int con_sock, int id){
    List *p = findClient(list, con_sock);
    p->ListUser.id = id;
}

void updateOtherClient(List *list, int con_sock, int other_sock){
    List *p = findClient(list, con_sock);
    p->ListUser.other_sock = con_sock;
}

void addLobby(List *list, int con_sock){
    List *p = findClient(list, con_sock);
    p->ListUser.lobby = true;
}

void removeLobby(List *list, int con_sock){
    List *p = findClient(list, con_sock);
    p->ListUser.lobby = false;
}

int getSize(List *list) {
    int total = 0;
    while(list!=NULL) {
        list = list->next;
        total++;
    }
    return total;
}

int getSizeLobby(List *list){
    int total = 0;
    while(list != NULL) {
        if(list->ListUser.lobby) total++;
        list = list->next;
    }
    return total;
}

List *Clients = NULL;