// Licence: Apache 2.0
// Copyright (2017): Dario Ostuni <dario.ostuni@gmail.com>

fn main()
{
    let pi = 3528.0 /
             (0..)
        .take(3)
        .map(|n: i64| {
            ((1..)
                .take(n as usize)
                .fold(1, |acc, _| -1 * acc) *
             (1..)
                .take(4 * n as usize)
                .product::<i64>() * (21460 * n + 1123)) as f64 /
            ((1..)
                .take(4)
                .fold(1,
                      |acc, _| {
                          (1..)
                              .take(n as usize)
                              .product::<i64>() * acc
                      }) *
             (1..)
                .take(4 * n as usize)
                .fold(1, |acc, _| 4 * acc) *
             (1..)
                .take(2 * n as usize)
                .fold(1, |acc, _| 882 * acc)) as f64
        })
        .sum::<f64>();
    println!("{}", pi);
}
