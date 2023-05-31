# RSA公钥加密体制

## 一 历史背景

1976年，两位美国计算机学家Whitfield Diffie 和 Martin Hellman，提出了一种崭新构思，可以在不直接传递密钥的情况下，完成解密。这被称为["Diffie-Hellman密钥交换算法"](https://en.wikipedia.org/wiki/Diffie–Hellman_key_exchange)。

![Whitfield.Diffie-Martin.Hellman](.\pic\Whitfield.Diffie-Martin.Hellman.jpg)

这个算法启发了其他科学家。人们认识到，加密和解密可以使用不同的规则，只要这两种规则之间存在某种对应关系即可，这样就避免了直接传递密钥。这种新的加密模式被称为"非对称加密算法"。随后，Merkle和Hellman利用这种思想，提出了基于背包问题的第一个可实现的公钥加密算法[MH(Merkle-Hellman)背包公钥密码算法](https://www.china.cx/post/2022/04/29/MH%E8%83%8C%E5%8C%85%E5%85%AC%E9%92%A5%E5%AF%86%E7%A0%81%E7%AE%97%E6%B3%95)。

> 　　（1）乙方生成两把密钥（公钥和私钥）。公钥是公开的，任何人都可以获得，私钥则是保密的。
>
> 　　（2）甲方获取乙方的公钥，然后用它对信息加密。
>
> 　　（3）乙方得到加密后的信息，用私钥解密。

如果公钥加密的信息只有私钥解得开，那么只要私钥不泄漏，通信就是安全的。

![Rivest-Shamir-Adleman](.\pic\Rivest-Shamir-Adleman.jpg)

随后1977年，三位数学家Rivest、Shamir 和 Adleman 设计了一种算法，可以实现非对称加密。这种算法用他们三个人的名字命名，叫做[RSA算法](https://zh.wikipedia.org/zh-cn/RSA加密算法)。

RSA虽然晚于MH背包公钥加密体制，但是它是第一安全、实用的公钥加密算法。已经成为国际标准，是目前最广为使用的"非对称加密算法"。

