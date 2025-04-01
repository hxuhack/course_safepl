struct List{
    val: u64,
    next: Option<Box<List>>,
}
impl List {
    fn new(val: u64) -> Self {
        List { val, next: None }
    } 
    fn prepend(self, val: u64) -> Self {
        List {
            val,
            next: Some(Box::new(self)),
        }
    }
    fn append(&mut self, val: u64) {
        let mut current = self;
        while let Some(ref mut next) = current.next {
            current = next;
        }
        current.next = Some(Box::new(List::new(val)));
    }
    fn print(&self) {
        let mut current = self;
        print!("{}", current.val);
        while let Some(ref next) = current.next {
            print!(" -> {}", next.val);
            current = next;
        }
        println!();
    }
}

fn main(){
    let mut l = List::new(1);
    l = l.prepend(0);
    l.append(2);
    l.print();
}
