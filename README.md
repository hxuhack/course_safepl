# Memory Safety and Programming Language Design
This is the course web page of COMP737011 Memory Safety and Programming Language Design for postgraduate students at Fudan University. The course will first go through the Foundations of Memory Safety Problem; then we will teach the Rust Programming Language by focusing on its special design for memory-safety protection; finally we will share some advanced topics useful for mitigating memory-safety issues.

## Lecture Notes
### Part1: Foundations of Memory Safety 

| Schedule | Topic | Materials  |
|---|---|---|
| Week1 | Course Introduction, Stack Smashing | [slides1](slides/L0-Course_Intro.pdf), [slides2](slides/L1-Stack_Smashing.pdf), [notes](notes/chapt1_stack_smash.md)|
| Week2 | Allocator Design | [slides](slides/L2-Allocator_Design.pdf), [notes](notes/chapt2_allocator_design.md), [code](code/chapt2-allocator-template.c) |
| Week3 | Heap Attack | [slides](slides/L2-Heap_Attack.pdf), [notes](notes/chapt3_heap_attack.md) |
| Week4 | Memory Exhaustion | [slides](slides/L4-Memory_Exhaustion.pdf), [notes](notes/chapt4_memory_exhaustion.md), [code](code/chapt4-stackoverflow_template.c) |
| Week5 | Auto Reclaim | [slides](slides/L5-Auto_Reclaim.pdf), [notes](notes/chapt5_auto_reclaim.md) |
| Week6 | Concurrency | [slides](slides/L6-Concurrency.pdf), [notes](notes/chapt6_concurrency.md) |
| Week7 | No Class |  |

### Part2: Rust Programming Language 
| Schedule | Topic | Materials  |
|---|---|---|
| Week8 | Rust Ownership-based Resource Management | [slides](slides/L7-Rust_OBRM.pdf), [notes](notes/chapt8_rust_obrm.md) |


### Reading List

**Empirical**
- Marcelo Almeida, Grant Cole, Ke Du, Gongming Luo, Shulin Pan, Yu Pan, Kai Qiu et al. "[RustViz: Interactively Visualizing Ownership and Borrowing](https://ieeexplore.ieee.org/abstract/document/9833121)." In 2022 IEEE Symposium on Visual Languages and Human-Centric Computing (VL/HCC), pp. 1-10. IEEE, 2022.
- Li, Hao, Filipe R. Cogo, and Cor-Paul Bezemer. "[An Empirical Study of Yanked Releases in the Rust Package Registry](https://ieeexplore.ieee.org/abstract/document/9714872)." IEEE Transactions on Software Engineering 49, no. 1 (2022): 437-449.
- Zhu, Shuofei, Ziyi Zhang, Boqin Qin, Aiping Xiong, and Linhai Song. "[Learning and programming challenges of rust: A mixed-methods study](https://dl.acm.org/doi/abs/10.1145/3510003.3510164)." In Proceedings of the 44th International Conference on Software Engineering, pp. 1269-1281. 2022.
- Xu, Hui, Zhuangbin Chen, Mingshen Sun, Yangfan Zhou, and Michael R. Lyu. "[Memory-safety challenge considered solved? An in-depth study with all Rust CVEs](https://dl.acm.org/doi/abs/10.1145/3466642)." ACM Transactions on Software Engineering and Methodology (TOSEM) 31, no. 1 (2021): 1-25.
- Kelsey R. Fulton, Anna Chan, Daniel Votipka, Michael Hicks, and Michelle L. Mazurek. "[Benefits and drawbacks of adopting a secure programming language: rust as a case study](https://www.usenix.org/system/files/soups2021-fulton.pdf)." In Symposium on Usable Privacy and Security. 2021.
- Qin, Boqin, Yilun Chen, Zeming Yu, Linhai Song, and Yiying Zhang. "[Understanding memory and thread safety practices and issues in real-world Rust programs](https://dl.acm.org/doi/pdf/10.1145/3385412.3386036)." In Proceedings of the 41st ACM SIGPLAN Conference on Programming Language Design and Implementation, pp. 763-779. 2020.
- Evans, Ana Nora, Bradford Campbell, and Mary Lou Soffa. "[Is Rust used safely by software developers?](https://ieeexplore.ieee.org/abstract/document/9283950)." In 2020 IEEE/ACM 42nd International Conference on Software Engineering (ICSE), pp. 246-257. IEEE, 2020.

**Application**
- Kevin Boos, Namitha Liyanage, Ramla Ijaz, and Lin Zhong. "[Theseus: an experiment in operating system structure and state management](https://www.usenix.org/conference/osdi20/presentation/boos)." In Proceedings of the 14th USENIX Conference on Operating Systems Design and Implementation, pp. 1-19. 2020.

**Static Analysis**
- Crichton, Will, Marco Patrignani, Maneesh Agrawala, and Pat Hanrahan. "[Modular information flow through ownership](https://dl.acm.org/doi/abs/10.1145/3519939.3523445)." In Proceedings of the 43rd ACM SIGPLAN International Conference on Programming Language Design and Implementation, pp. 1-14. 2022.
- Cui, Mohan, Chengjun Chen, Hui Xu, and Yangfan Zhou. "[SafeDrop: Detecting memory deallocation bugs of rust programs via static data-flow analysis](https://dl.acm.org/doi/abs/10.1145/3542948)." ACM Transactions on Software Engineering and Methodology (2022).
- Yechan Bae, Youngsuk Kim, Ammar Askar, Jungwon Lim, and Taesoo Kim. "[Rudra: Finding Memory Safety Bugs in Rust at the Ecosystem Scale](https://dl.acm.org/doi/pdf/10.1145/3477132.3483570)." In Proceedings of the ACM SIGOPS 28th Symposium on Operating Systems Principles, pp. 84-99. 2021.
- Zhuohua Li, Jincheng Wang, Mingshen Sun, and John CS Lui. "[MirChecker: Detecting Bugs in Rust Programs via Static Analysis](https://dl.acm.org/doi/pdf/10.1145/3460120.3484541)." In Proceedings of the 2021 ACM SIGSAC Conference on Computer and Communications Security, pp. 2183-2196. 2021.
- Popescu, Natalie, Ziyang Xu, DAVID I. AUGUST, and AMIT LEVY. "[Safer at any speed: automatic context-aware safety enhancement for Rust](http://www.amitlevy.com/papers/nader-oopsla21.pdf)." Proceedings of the ACM on Programming Languages 5, no. OOPSLA (2021): 103.

**Testing, Dynamic Analysis**
- VanHattum, Alexa, Daniel Schwartz-Narbonne, Nathan Chong, and Adrian Sampson. "[Verifying Dynamic Trait Objects in Rust](https://www.cs.cornell.edu/~avh/dyn-trait-icse-seip-2022-preprint.pdf)." ICSE-SEIP, 2022.
- Takashima, Yoshiki, Ruben Martins, Limin Jia, and Corina S. Păsăreanu. "[Syrust: automatic testing of rust libraries with semantic-aware program synthesis](https://dl.acm.org/doi/abs/10.1145/3453483.3454084)." In Proceedings of the 42nd ACM SIGPLAN International Conference on Programming Language Design and Implementation, pp. 899-913. 2021.
- Jianfeng Jiang, Hui Xu, and Yangfan Zhou. "[RULF: Rust library fuzzing via API dependency graph traversal](https://ieeexplore.ieee.org/abstract/document/9678813/)." In 2021 36th IEEE/ACM International Conference on Automated Software Engineering (ASE), pp. 581-592. IEEE, 2021.


**Formal Method**
- Lattuada, Andrea, Travis Hance, Chanhee Cho, Matthias Brun, Isitha Subasinghe, Yi Zhou, Jon Howell, Bryan Parno, and Chris Hawblitzel. "[Verus: Verifying Rust Programs using Linear Ghost Types](https://dl.acm.org/doi/abs/10.1145/3586037)." Proceedings of the ACM on Programming Languages 7, no. OOPSLA1 (2023): 286-315.
- Ho, Son, and Jonathan Protzenko. "[Aeneas: Rust verification by functional translation](https://dl.acm.org/doi/pdf/10.1145/3547647)." Proceedings of the ACM on Programming Languages 6, no. ICFP (2022): 711-741.
- Vytautas Astrauskas, Aurel Bílý, Jonáš Fiala, Zachary Grannan, Christoph Matheja, Peter Müller, Federico Poli, and Alexander J. Summers. "[The prusti project: Formal verification for Rust](https://dl.acm.org/doi/10.1007/978-3-031-06773-0_5)." In NASA Formal Methods: 14th International Symposium, NFM 2022, Pasadena, CA, USA, May 24–27, 2022, Proceedings, pp. 88-108. Cham: Springer International Publishing, 2022.
- Fabian Wolff, Aurel Bílý, Christoph Matheja, Peter Müller, and Alexander J. Summers. "[Modular specification and verification of closures in Rust](https://dl.acm.org/doi/abs/10.1145/3485522)." Proceedings of the ACM on Programming Languages 5, no. OOPSLA (2021): 1-29.
- Matsushita, Yusuke, Takeshi Tsukada, and Naoki Kobayashi. "[RustHorn: CHC-Based Verification for Rust Programs](https://library.oapen.org/bitstream/handle/20.500.12657/37721/2020_Book_ProgrammingLanguagesAndSystems.pdf?sequence=1#page=498)." In ESOP, pp. 484-514. 2020.
- Dang, Hoang-Hai, Jacques-Henri Jourdan, Jan-Oliver Kaiser, and Derek Dreyer. "[RustBelt meets relaxed memory](https://dl.acm.org/doi/pdf/10.1145/3371102)." Proceedings of the ACM on Programming Languages 4, no. POPL (2019): 1-29.
- Jung, Ralf, Hoang-Hai Dang, Jeehoon Kang, and Derek Dreyer. "[Stacked borrows: an aliasing model for Rust](https://dl.acm.org/doi/pdf/10.1145/3371109)." Proceedings of the ACM on Programming Languages 4, no. POPL (2019): 1-32.
- Ralf Jung, Jacques-Henri Jourdan, Robbert Krebbers, and Derek Dreyer. "[RustBelt: Securing the foundations of the Rust programming language](https://dl.acm.org/doi/pdf/10.1145/3158154)." Proceedings of the ACM on Programming Languages 2, no. POPL (2017): 1-34.

**Isolation**
- Rivera, Elijah, Samuel Mergendahl, Howard Shrobe, Hamed Okhravi, and Nathan Burow. "[Keeping Safe Rust Safe with Galeed](https://dl.acm.org/doi/fullHtml/10.1145/3485832.3485903)." In Annual Computer Security Applications Conference, pp. 824-836. 2021.
- Peiming Liu, Gang Zhao, and Jeff Huang. "[Securing unsafe rust programs with XRust](https://dl.acm.org/doi/pdf/10.1145/3377811.3380325)." In Proceedings of the ACM/IEEE 42nd International Conference on Software Engineering, pp. 234-245. 2020.
- Benjamin Lamowski, Carsten Weinhold, Adam Lackorzynski, and Hermann Härtig. "[Sandcrust: Automatic sandboxing of unsafe components in rust](https://dl.acm.org/doi/pdf/10.1145/3144555.3144562)." In Proceedings of the 9th Workshop on Programming Languages and Operating Systems, pp. 51-57. 2017.

**Other Language**
- Jaemin Hong, and Sukyoung Ryu. "[Concrat: An Automatic C-to-Rust Lock API Translator for Concurrent Programs](https://arxiv.org/abs/2301.10943)." ICSE'2023.
- Emre, Mehmet, Ryan Schroeder, Kyle Dewey, and Ben Hardekopf. "[Translating C to safer Rust](https://dl.acm.org/doi/pdf/10.1145/3485498)." Proceedings of the ACM on Programming Languages 5, no. OOPSLA (2021): 1-29.
- Sammler, Michael, Rodolphe Lepigre, Robbert Krebbers, Kayvan Memarian, Derek Dreyer, and Deepak Garg. "[RefinedC: automating the foundational verification of C code with refined ownership types](https://dl.acm.org/doi/pdf/10.1145/3453483.3454036)." In Proceedings of the 42nd ACM SIGPLAN International Conference on Programming Language Design and Implementation, pp. 158-174. 2021.
