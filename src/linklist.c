#include "linklist.h"

int is_singlelinklist_empty(single_linklist_head *head)
{
	if(head->linknode_count == 0){
		return 1;
	}else{
		return 0;
	}
}

void singlelinklist_pop_node(single_linklist_head *head, single_linklist *datanode)
{
	if(datanode == NULL || head == NULL){
		err("single_linklist node is NULL");
		return ;
	}
	if(head->linknode_count == 0){
		err("single_linklist is empyt");
		return;
	}
	if(datanode->data == NULL){
		err("datanode buffer is NULL");
		return;
	}

    pthread_mutex_lock(&(head->mutex));	
	memcpy(datanode->data->pdata, head->linklist_head->next->data->pdata, head->linklist_head->next->data->data_len);
	datanode->data->data_len = head->linklist_head->next->data->data_len;
	datanode->data->recv_fd = head->linklist_head->next->data->recv_fd;
	datanode->data->send_to = head->linklist_head->next->data->send_to;

	single_linklist *tmp = head->linklist_head->next;
	head->linklist_head->next = head->linklist_head->next->next;
	head->linknode_count--;
	if(head->linknode_count == 0){
		head->linklist_tail = head->linklist_head;
		head->linklist_tail->next = head->linklist_head;
	}
	pthread_mutex_unlock(&(head->mutex));
	singlelinklist_destory_node(tmp);
}

single_linklist *singlelinklist_init_node(int buf_len)
{
	single_linklist *node = (single_linklist *)calloc(1, sizeof(single_linklist));
	if(node == NULL){
		err("linklist calloc failed");
		return NULL;
	}
	node->next = NULL;
	node->data = (data_inf *)calloc(1, sizeof(data_inf));
	if(node->data == NULL){
		err("data calloc failed");
		goto fail;
	}

	node->data->pdata = (char *)calloc(1, buf_len);
	if(node->data->pdata == NULL){
		err("buf calloc failed");
		goto fail;
	}
	node->data->data_len = 0;
	node->data->send_to = 0;
	return node;

fail:
	if(node->data != NULL){
		if(node->data->pdata != NULL){
			free(node->data->pdata);
			node->data->pdata = NULL;
		}
		free(node->data);
		node->data = NULL;
	}
	free(node);
	node = NULL;
	return NULL;
}

void singlelinklist_destory_node(single_linklist *datanode)
{
	if(datanode == NULL){
		err("single_linklist node is NULL");
		return;
	}
	if(datanode->data == NULL){
		err("data is NULL");
	}else{
		if(datanode->data->pdata == NULL){
			err("buf is NULL");
		}else{
			free(datanode->data->pdata);
			datanode->data->pdata = NULL;
		}
		free(datanode->data);
		datanode->data = NULL;
	}
	free(datanode);	
	datanode = NULL;
}

void singlelinklist_insert_node(single_linklist_head *head, single_linklist *datanode)
{
	if(datanode == NULL || head == NULL ){
		err("single_linklist node is NULL");
		return ;
	}
    pthread_mutex_lock(&(head->mutex));	
	head->linklist_tail->next = datanode;
	head->linklist_tail = datanode;
	datanode->next = head->linklist_tail;
	head->linknode_count++;
	pthread_mutex_unlock(&(head->mutex));
#if 0
	int i = 0;
	single_linklist *tmp = head->linklist_head->next;
	for(; i < head->linknode_count; i++){
		printf("%d, %s, %p\n", __LINE__, tmp->data->buf, tmp->data->buf);
		tmp = tmp->next;
	}
	if(tmp == head->linklist_tail){
		printf("%d, %s\n", __LINE__, tmp->data->buf);
	}else{
		printf("%d\n", __LINE__);
	}
#endif
}

void destory_singlelinklist(single_linklist_head *head)
{
	if(head == NULL){
		err("linklist is not exist");
		return;
	}
	if(head->linklist_head == NULL){
		err("linklist is not exist");
		return;
	}
	single_linklist *tmp = NULL;
	
    pthread_mutex_lock(&(head->mutex));		
	while(head->linknode_count > 0){
		tmp = head->linklist_head->next;
		printf("destorying [%d]:[%s]\n", head->linknode_count, tmp->data->pdata);
		head->linklist_head->next = head->linklist_head->next->next;
		head->linknode_count--;
		singlelinklist_destory_node(tmp);
		if(head->linknode_count == 0){
			head->linklist_tail = head->linklist_head;
			head->linklist_tail->next = head->linklist_head;
			singlelinklist_destory_node(head->linklist_head);
		}
	}
	pthread_mutex_unlock(&(head->mutex));
    pthread_mutex_destroy(&(head->mutex));          //清理资源
	free(head);
	head = NULL;
}

single_linklist_head *init_singlelinklist()
{
	single_linklist *linklist_head = (single_linklist *)calloc(1, sizeof(single_linklist));
	if(linklist_head == NULL){
		err("linklist calloc failed");
		return NULL;
	}
	linklist_head->next = NULL;
	linklist_head->data = NULL;

	single_linklist_head *head = (single_linklist_head *)calloc(1, sizeof(single_linklist_head));
	if(head == NULL){
		err("linklist calloc failed");
		return NULL;
	}
	head->linklist_head = linklist_head;
	head->linklist_tail = linklist_head;
	linklist_head->next = head->linklist_tail;
	head->linknode_count = 0;	
	if (pthread_mutex_init(&(head->mutex), NULL))
	{
		printf("failed to init mutex!\n");
		return NULL;
	}
	return head;
}

int add_data_to_linklist(single_linklist_head *head, data_inf *data)	
{
	single_linklist *sock_linknode = NULL;	

	if((sock_linknode = singlelinklist_init_node(data->data_len)) == NULL){
		return -1;
	}
	sock_linknode->data->recv_fd = data->recv_fd;
	sock_linknode->data->data_len = data->data_len;
	memcpy(sock_linknode->data->pdata, data->pdata, data->data_len);
	singlelinklist_insert_node(head, sock_linknode);
	return 0;
}

int test()
{
	single_linklist_head *head = init_singlelinklist();
	single_linklist *record[1024] = {NULL};
	int i = 0;
	while(1){
		single_linklist *node = singlelinklist_init_node(1024);
		if(node != NULL){
			record[i++] = node;
		}else{
			return -1;
		}
		memset(node->data->pdata, 0, 1024);
		fgets(node->data->pdata, 1024, stdin);
		if(!strncmp("pop", node->data->pdata, 3)){
			single_linklist *node2 = singlelinklist_init_node(1024);
			singlelinklist_pop_node(head, node2);
			printf("%d node count: %d\n", __LINE__, head->linknode_count);
			printf("node2:len: [%d] %s\n", node2->data->data_len, node2->data->pdata);
			singlelinklist_destory_node(node2);
			continue;
		}else if(!strncmp("quit", node->data->pdata, 4)){
			destory_singlelinklist(head);
			break;
		}
		node->data->data_len = strlen(node->data->pdata);
		singlelinklist_insert_node(head, node);
		printf("%d node count: %d\n", __LINE__, head->linknode_count);
		
	}
}

