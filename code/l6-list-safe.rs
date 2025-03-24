use std::cell::RefCell;
use std::rc::{Rc, Weak};

struct Node {
    val: u64,
    prev: Option<Weak<RefCell<Node>>>,
    next: Option<Rc<RefCell<Node>>>,
}

struct List {
    head: Option<Rc<RefCell<Node>>>,
    tail: Option<Rc<RefCell<Node>>>,
}

impl List {
    fn new() -> Self {
        Self { head: None, tail: None }
    }

    pub fn append(&mut self, val: u64) {
        let new_node = Rc::new(RefCell::new(Node {
            val,
            prev: self.tail.as_ref().map(|tail| Rc::downgrade(tail)),
            next: None,
        }));

        match self.tail.take() {
            Some(old_tail) => {
                old_tail.borrow_mut().next = Some(Rc::clone(&new_node));
                self.tail = Some(new_node);
            }
            None => {
                self.head = Some(Rc::clone(&new_node));
                self.tail = Some(new_node);
            }
        }
    }

    pub fn prepend(&mut self, val: u64) {
        let new_node = Rc::new(RefCell::new(Node {
            val,
            prev: None,
            next: self.head.clone(),
        }));

        match self.head.take() {
            Some(old_head) => {
                old_head.borrow_mut().prev = Some(Rc::downgrade(&new_node));
                self.head = Some(Rc::clone(&new_node));
            }
            None => {
                self.head = Some(Rc::clone(&new_node));
                self.tail = Some(new_node);
            }
        }
    }

    pub fn print(&self) {
        let mut current = self.head.clone();
        while let Some(node) = current {
            print!("{} ", node.borrow().val);
            current = node.borrow().next.clone();
        }
        println!();
    }
}

fn main() {
    let mut list = List::new();
    list.append(1);
    list.append(2);
    list.append(3);
    println!("After append:");
    list.print();

    list.prepend(0);
    println!("After prepend:");
    list.print();
}

