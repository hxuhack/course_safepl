use std::cell::Cell;
use std::cell::RefCell;
use std::sync::Mutex;
use std::sync::Arc;
use std::marker::PhantomData;
use std::sync::MutexGuard;
//use std::thread;

struct MyT<'a> {p:&'a Box<i32>,}
//struct MyT {p:*mut i32,}

fn check_sync<T: Sync>(_: T) {} 
fn check_send<T: Send>(_: T) {} 

fn testCell(){
    let mut v = Cell::new(1);
    //check_send(v); // success
    //check_send(&mut v); // success
    //check_send(&v); //fail
    //check_sync(v); //fail
}


fn testRefCell(){
    let mut v = RefCell::new(1);
    //check_send(v); // success
    check_send(&mut v); // success
    //check_send(&v); //fail
    //check_sync(v); //fail
}

fn testMutex1(){
    let mut v = Mutex::new(1);
    //check_send(v); // success
    //check_send(&v); // success
    //check_send(&mut v); // success
    //check_sync(&v); // success
    check_sync(&mut v);// success
}

fn testMutex2(){
    let mut v = Mutex::new(Cell::new(1));
    //check_send(v); // success
    //check_send(&v); // success
    //check_send(&mut v); // success
    check_sync(v);// success
}

fn testMutex3(){
    let mut v = Mutex::new(&Cell::new(1));
    //check_send(v); // fail 
    //check_sync(v); // fail 
}

fn testMutex4(){
    let mut cell = Cell::new(1);
    //let mut v = Mutex::new(cell);
    //check_send(v); // success
    //check_sync(v); // success
}
fn testArc(){
    /*
    let mut cell = Cell::new(1);
    let mut v = Arc::new(&cell);
    //let mut v = Arc::new(cell);
    let v1 = v.clone();
    thread::spawn(move || {
        (*v1).set(3);
    }).join();
    (*v).set(2);
    */
}
fn testMutexGuard(){
    let v = PhantomData::<MutexGuard<i32>>{};
    //check_send(v); //fail
    check_sync(&v); //success
}

fn main() { 
    //let mut x = Box::new(1);
    //check_send(&mut x);
    //check_sync(x);
}


