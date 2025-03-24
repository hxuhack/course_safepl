use std::ptr;

fn owner(){
    let mut alice = 1;
    alice += 1;
    println!("alice = {}", alice);
}

fn borrow(){
    let mut alice = 1;
    let mut carol = 2;
    let mut bob = &mut alice;
    *bob += 1;
    bob = &mut carol;
    *bob += 1;
    println!("alice = {}", alice);
    println!("carol = {}", carol);
}

fn main(){
    owner();
    borrow();
}
