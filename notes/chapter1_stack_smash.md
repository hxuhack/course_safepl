# Chapter 1. Stack Smashing
In this chapter, we discuss the most common and easy-to-understand memory-safety problems: stack buffer overflow. An attacker could leverage such bugs to modity the stack and achieve malicious goals, knowning as stack smashing. We will not dive into the technical details of exploit writting (there are many such tutorials and tools available online, e.g., for CTF training) but mainly discuss why such bugs are dangerous and show how attacks could happen in practice. 

## Section 1. Stack Smashing
'''
int validation() {
    char buf[64];
    read(STDIN_FILENO, buf, 160);
    if(buf[0]=='$'){
        write(STDOUT_FILENO, "Key verified!\n", 14);
        return 1;
    }else{
    	write(STDOUT_FILENO, "Wrong key!\n", 11);
    }
    return 0;
}
int main(int argc, char** argv){
    int flag = 0;
    while(!flag){
    	write(STDOUT_FILENO, "Input your key:", 15);
        flag = validation();
    }
    printf("Start...\n");
}
'''

## Section 2. Protection Techniques
