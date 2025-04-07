use std::thread;

#[derive(Debug)]
struct Unsend{
    ptr: *mut i64,
}
impl Unsend{
    fn add(&self, i:i64){
        unsafe{*(self.ptr) = *self.ptr + i};
    }
}
unsafe impl Send for Unsend{}

fn main(){
    let mut var = 0i64; 
    let v = Unsend{ptr:&mut var as *mut i64};
    let tid = thread::spawn(move || {
        for i in 1..100001{
            v.add(i);
        }
    });
    for i in 1..100001{
        var+=i;
    }
    let _ = tid.join();
    println!("{}",var);
}
