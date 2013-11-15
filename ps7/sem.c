/* Mark Bryk OS PS7 SEM.C */

void sem_init(struct sem *s, int count){
	s->lock = 0;
	s->count = count;
	memset(s->queue, 0, QSIZE);
}

int sem_try(struct sem *s){
	int l = 0;
	while(tas(&s->lock)!=0);
	if(s->count==1){
		l = 1;
		s->count--;
	}
	s->lock = 0;
	return l;		
}

void sem_wait(struct sem *s){
	s->count--;
	while(1){
		while(tas(&s->lock)!=0); // Grab lock for count check
		if(s->count < 0){ // Somebody is using semaphore
			s->lock = 0; // Release lock
			add to end of queue;
			sleep;
			continue;	
		}
		s->lock = 0;
		break; //Now you have lock and semaphore says that you're good
	}
}
void sem_inc(struct sem *s){
	s->count++;
	while(tas(&s->lock)!=0); // Grab lock for count check
	int c = s->count;
	s->lock = 0;
	if(c <= 0){ // There are people waiting
		wakeup(queue[0]);
	}
}
