
fn testmove(){
    let mut alice = Box::new(1);
    let bob = alice;
    println!("bob:{}", bob);
    //println!("alice:{}", alice);

}

fn testclone(){
    let mut alice = Box::new(1);
    let bob = alice.clone();
    println!("bob:{}", bob);
    println!("alice:{}", alice);
}

fn testcopy(){
    let mut alice = 1;
    let bob = alice;
    println!("bob:{}", bob);
    println!("alice:{}", alice);
}

fn main() {
    testmove();     
    testclone();     
    testcopy();     
}
