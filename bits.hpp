#ifndef BIT_HPP
#define BIT_HPP

/* Bit abstraction library
 * Author:  Yago T. de Mello
 * e-mail:  yago.t.mello@gmail.com
 * Version: 2.6.0 2021-11-24
 * License: Apache 2.0
 * C++20
 */

/*
Copyright 2021 Yago Teodoro de Mello

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
*/

#include <type_traits>

namespace bits {

// Used by bits::make_bit to select the correct template
// when reg_type is a pointer
template <typename T>
concept is_not_pointer = !std::is_pointer<typename std::remove_reference<T>::type>::value;

template <typename T>
concept is_const = std::is_const<typename std::remove_pointer<typename std::remove_reference<T>::type>::type>::value;

template <typename T>
concept is_not_const = !is_const<T>;

class bit {
public:
    // The mutator and accessor function-pointers
    using func_write_type = void (*)(const bool value);
    using func_read_type = bool (*)();
    
    // Allows the creation of uninitialized instances
    bit() = default;
    
    // Used by bits::make_bit
    bit(
        func_write_type func_write,
        func_read_type func_read
    ) noexcept {
        func_write_ = func_write;
        func_read_ = func_read;
    }
    
    // Copy constructor
    // Copies the value but not the function pointers
    bit(const bit & obj) {
        this->write(obj.read());
    }
    
    // Sets the bit value
    constexpr bit & operator =(const bool value) {
        this->func_write_(value);
        return *this;
    }
    
    // Forbid copy assignment
    // forcing conversion to bool
    bit & operator =(const bit &) = delete;
    
    // Forces this object to use the other object's
    // function pointers
    constexpr void same_as(const bit & obj) noexcept {
        this->func_write_ = obj.func_write_;
        this->func_read_ = obj.func_read_;
    }
    
    // Gets the bit value
    constexpr operator bool() const {
        return this->func_read_();
    }
    
    // Explicit way to set the value to true
    constexpr void set() {
        this->func_write_(true);
    }
    
    // Explicit way to set the value to false
    constexpr void clr() {
        this->func_write_(false);
    }
    
    // Explicit way to set the value
    constexpr void write(const bool value) {
        this->func_write_(value);
    }
    
    // Explicit way to read the value
    [[nodiscard]] constexpr bool read() const {
        return this->func_read_();
    }
    
    // Template polymorphism generator
    // Writes the value to a compile-time known variable
    // If the variable is not const
    template <auto & reg, auto offset>
    static constexpr void default_write_static(const bool value) 
    requires is_not_const<decltype(reg)> {
        using reg_type = std::remove_reference<decltype(reg)>::type;
        using value_type = std::remove_volatile<reg_type>::type;
        
        // Bit masks to select/clear the bit
        constexpr value_type mask_bit = value_type(1) << offset;
        constexpr value_type mask_clear = ~mask_bit;
        
        // The clear result is stored in a temporary
        // to prevent unnecessary writes/reads to reg
        // when reg is volatile
        const value_type reg_clr = reg & mask_clear;
        reg = reg_clr | value_type(value_type(value) * mask_bit);
    }
    
    // Template polymorphism generator
    // If the variable is const, do not write to it
    template <auto & reg, auto offset>
    static constexpr void default_write_static(const bool) 
    requires is_const<decltype(reg)> { }
    
    // Template polymorphism generator
    // Reads the value of a compile-time known variable
    template <auto & reg, auto offset>
    static constexpr bool default_read_static() {
        using reg_type = std::remove_reference<decltype(reg)>::type;
        using value_type = std::remove_volatile<reg_type>::type;
        
        // Bit mask to select the bit
        constexpr value_type mask_bit = value_type(1) << offset;
        
        return (reg & mask_bit) != value_type(0);
    }
    
    // Template polymorphism generator
    // Writes the value to a variable in a compile-time known pointer
    template <auto * & reg_ptr, auto offset>
    static constexpr void default_write_dynamic(const bool value) 
    requires is_not_const<decltype(reg_ptr)> {
        using ptr_type = std::remove_reference<decltype(reg_ptr)>::type;
        using reg_type = std::remove_pointer<ptr_type>::type;
        using value_type = std::remove_volatile<reg_type>::type;
        
        // Bit masks to select/clear the bit
        constexpr value_type mask_bit = value_type(1) << offset;
        constexpr value_type mask_clear = ~mask_bit;
        
        // The clear result is stored in a temporary
        // to prevent unnecessary writes/reads to reg
        // when reg is volatile
        const value_type reg_clr = *reg_ptr & mask_clear;
        *reg_ptr = reg_clr | value_type(value_type(value) * mask_bit);
    }
    
    template <auto & reg, auto offset>
    static constexpr void default_write_dynamic(const bool) 
    requires is_const<decltype(reg)> { }
    
    // Template polymorphism generator
    // Reads the value of a variable in a compile-time known pointer
    template <auto * & reg_ptr, auto offset>
    static constexpr bool default_read_dynamic() {
        using ptr_type = std::remove_reference<decltype(reg_ptr)>::type;
        using reg_type = std::remove_pointer<ptr_type>::type;
        using value_type = std::remove_volatile<reg_type>::type;
        
        // Bit mask to select the bit
        constexpr value_type mask_bit = value_type(1) << offset;
        
        return (*reg_ptr & mask_bit) != value_type(0);
    }
    
private:
    // The function pointers
    func_write_type func_write_;
    func_read_type func_read_;
};

// Creates a bit object to access a compile-time known variable
template <auto & reg, auto offset>
bits::bit make_bit(
    bit::func_write_type func_write = &bit::default_write_static<reg, offset>,
    bit::func_read_type func_read = &bit::default_read_static<reg, offset>
) requires is_not_pointer<decltype(reg)> {
    return bits::bit(func_write, func_read);
}

// Creates a bit object to access a variable in a compile-time known pointer
template <auto * & reg_ptr, auto offset>
bits::bit make_bit(
    bit::func_write_type func_write = &bit::default_write_dynamic<reg_ptr, offset>,
    bit::func_read_type func_read = &bit::default_read_dynamic<reg_ptr, offset>
) {
    return bits::bit(func_write, func_read);
}

} // namespace bits

#endif // BIT_HPP 
