use std::ptr;

#[derive(Debug)]
struct Node {
    val: i32,
    next: *mut Node,
    prev: *mut Node,
}

#[derive(Debug)]
struct List{
    head: *mut Node,
    tail: *mut Node,
}

impl List {
    pub fn new() -> Self { List {head:ptr::null_mut(), tail:ptr::null_mut()} }
    pub fn push(&mut self, value: i32) {
        let new = Box::new(Node {val: value, prev:ptr::null_mut(), next:ptr::null_mut()});
        let raw = Box::into_raw(new);
        unsafe {
            (*raw).next = self.head;
            if !self.head.is_null() {
                (*self.head).prev = raw;
            } else {
                self.tail = raw;
            }
            self.head = raw;
        }
    }
}

fn main(){
    let mut l = List::new();
    l.push(1);
    l.push(2);
    l.push(3);
    println!("{:?}", l);
}
