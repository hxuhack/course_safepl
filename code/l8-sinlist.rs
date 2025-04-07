use std::fmt;
use std::sync::{Arc, Mutex};
use std::thread;

#[derive(Debug)]
struct List<T> {
    val: T,
    next: Option<Box<List<T>>>,
}

impl<T: Send> List<T> {
    fn new(val: T) -> Self {
        List { val, next: None }
    }

    fn prepend(self, val: T) -> Self {
        List {
            val,
            next: Some(Box::new(self)),
        }
    }

    fn append(&mut self, val: T) {
        let mut current = self;
        while let Some(ref mut next) = current.next {
            current = next;
        }
        current.next = Some(Box::new(List::new(val)));
    }

    fn print(&self) 
    where
        T: fmt::Display,
    {
        let mut current = self;
        print!("{}", current.val);
        while let Some(ref next) = current.next {
            print!(" -> {}", next.val);
            current = next;
        }
        println!(" -> nil");
    }
}

impl<T: fmt::Display> fmt::Display for List<T> {
    fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        write!(f, "{}", self.val)?;
        if let Some(ref next) = self.next {
            write!(f, " -> {}", next)?;
        }
        Ok(())
    }
}

#[derive(Clone)]
struct ThreadSafeList<T> {
    inner: Arc<Mutex<List<T>>>,
}

impl<T: Send + 'static> ThreadSafeList<T> {
    fn new(val: T) -> Self {
        Self {
            inner: Arc::new(Mutex::new(List::new(val))),
        }
    }

    fn append(&self, val: T) {
        let mut list = self.inner.lock().unwrap();
        list.append(val);
    }

    fn print(&self)
    where
        T: fmt::Display,
    {
        let list = self.inner.lock().unwrap();
        list.print();
    }
}

fn main() {
    let list = ThreadSafeList::new(0);

    let mut handles = vec![];

    for i in 1..6 {
        let list_clone = list.clone();
        let handle = thread::spawn(move || {
            list_clone.append(i);
        });
        handles.push(handle);
    }

    for handle in handles {
        handle.join().unwrap();
    }

    list.print();
}

