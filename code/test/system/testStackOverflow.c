void fun(){
    int i = 1;
    if (i){
        fun();
    }
}

int main(){
    fun();
    return 0;
    *(int *) fun = 0;
}




