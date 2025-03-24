fn longer<'a:'b,'b>(x:&'a String, y:&'b String)->&'b String{
    if x.len()>y.len(){
        x
    } else {
        y
    }
}

fn main(){
    let str1 = String::from("alice");
    let result;
    //{
        let str2 = String::from("bob111");
        result = longer(&str1, &str2);
    //}
    println!("The longer string is {}", result);
}
