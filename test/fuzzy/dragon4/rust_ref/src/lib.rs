#![feature(flt2dec)]
#![feature(maybe_uninit_uninit_array)]

extern crate core;

use core::num::flt2dec::strategy::dragon::{format_exact, format_shortest};
use core::num::flt2dec::MAX_SIG_DIGITS;

use core::num::flt2dec::decode;
use core::num::flt2dec::decoder::FullDecoded;
use std::mem::MaybeUninit;
use std::ptr::null_mut;

#[repr(C)]
struct Buffer {
    k: i16,
    len: usize,
    data: *mut u8,
}

#[no_mangle]
extern "C" fn rust_shortest(d: f64) -> Buffer {
    let (_sign, full_decoded) = decode(d);
    match full_decoded {
        FullDecoded::Finite(decoded) => {
            let mut buf: [MaybeUninit<u8>; MAX_SIG_DIGITS] = MaybeUninit::uninit_array();
            let (buf, exp) = format_shortest(&decoded, &mut buf);

            let mut vec = buf.to_vec();
            let data = vec.as_mut_ptr();
            let len = vec.len();
            std::mem::forget(vec);
            Buffer { k: exp, len, data }
        }
        _ => Buffer {
            k: 0,
            len: 0,
            data: null_mut(),
        },
    }
}

#[no_mangle]
extern "C" fn rust_fixed(d: f64, precision: i16) -> Buffer {
    let (_sign, full_decoded) = decode(d);
    match full_decoded {
        FullDecoded::Finite(decoded) => {
            let mut buf: [MaybeUninit<u8>; 1024] = MaybeUninit::uninit_array();
            let (buf, exp) = format_exact(&decoded, &mut buf, -precision);

            let mut vec = buf.to_vec();
            let data = vec.as_mut_ptr();
            let len = vec.len();
            std::mem::forget(vec);
            Buffer { k: exp, len, data }
        }
        _ => Buffer {
            k: 0,
            len: 0,
            data: null_mut(),
        },
    }
}

#[no_mangle]
extern "C" fn rust_exponent(d: f64, limit: i16) -> Buffer {
    let (_sign, full_decoded) = decode(d);
    match full_decoded {
        FullDecoded::Finite(decoded) => {
            let mut buf: [MaybeUninit<u8>; 1024] = MaybeUninit::uninit_array();
            let mut buf = &mut buf[..limit as usize];
            let (buf, exp) = format_exact(&decoded, &mut buf, i16::MIN);

            let mut vec = buf.to_vec();
            let data = vec.as_mut_ptr();
            let len = vec.len();
            std::mem::forget(vec);
            Buffer { k: exp, len, data }
        }
        _ => Buffer {
            k: 0,
            len: 0,
            data: null_mut(),
        },
    }
}

#[no_mangle]
extern "C" fn rust_free(buf: Buffer) {
    if buf.len == 0 {
        return;
    }
    let s = unsafe { std::slice::from_raw_parts_mut(buf.data, buf.len as usize) };
    let s = s.as_mut_ptr();
    unsafe {
        let _ = Box::from_raw(s);
    }
}

#[cfg(test)]
mod tests {
    use super::*;
    use std::str;

    #[test]
    fn it_works() {
        let b = rust_fixed(0.1, 40);
        let s = unsafe { std::slice::from_raw_parts_mut(b.data, b.len as usize) };
        println!("{}; {}", str::from_utf8(s).unwrap(), b.k);
        rust_free(b);
    }
}
