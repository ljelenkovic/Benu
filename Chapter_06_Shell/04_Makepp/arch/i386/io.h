/*! Input output utility functions for devices that require 'in' and 'out'
 *  instructions
 */

#pragma once

#include <types/basic.h>

/*!
 * Write to 8-bit port
 * \param port	Port number
 * \param data	Data to be sent
 */
static inline void outb ( uint16 port, uint8 data )
{
	asm ( "outb %b0, %w1" : : "a" (data), "d" (port) );
}

/*!
 * Read from 8-bit port
 * \param port	Port number
 * \return Read data
 */
static inline uint8 inb ( uint16 port )
{
	uint8 r;

	asm volatile ( "inb %w1, %b0" : "=a" (r) : "d" (port) );

	return r;
}

/*!
 * Write to 16-bit port
 * \param port	Port number
 * \param data	Data to be sent
 */
static inline void outw ( uint16 port, uint16 data )
{
	asm ( "outw %w0, %w1" : : "a" (data), "d" (port) );
}

/*!
 * Read from 16-bit port
 * \param port	Port number
 * \return Read data
 */
static inline uint16 inw ( uint16 port )
{
	uint16 r;

	asm volatile ( "inw %w1, %w0" : "=a" (r) : "d" (port) );

	return r;
}

/*!
 * Write to 32-bit port
 * \param port	Port number
 * \param data	Data to be sent
 */
static inline void outl ( uint16 port, uint32 data )
{
	asm ( "outl %0, %1" : : "a" (data), "d" (port) );
}

/*!
 * Read from 32-bit port
 * \param port	Port number
 * \return Read data
 */
static inline uint32 inl ( uint16 port )
{
	uint32 r;

	asm volatile ( "inl %1, %0" : "=a" (r) : "d" (port) );

	return r;
}

/*
inline unsigned long inportl(unsigned short port)
{
   unsigned long result;
   __asm__ __volatile__("inl %%dx, %%eax" : "=a" (result) : "dN" (port));
   return result;
}

inline void outportl(unsigned short port, unsigned long data)
{
   __asm__ __volatile__("outl %%eax, %%dx" : : "d" (port), "a" (data));
}
*/



/*!
 * Write string to 16-bit port
 * \param port	Port number
 * \param data	String to be sent
 * \param size	Size of Data
 */
static inline void outsw ( uint16 port, const void *data, uint16 size )
{
	asm volatile (	"cld\n\t"
			"rep outsw\n\t"
			: "+S" (data), "+c" (size) : "d" (port) );
}

/*!
 * Read string from 16-bit port
 * \param port	Port number
 * \param data	Address where to store read string
 * \param size	Size of Data
 */
static inline void insw ( uint16 port, void *data, uint16 size )
{
	asm volatile (	"cld\n\t"
			"rep insw\n\t"
			: "+D" (data), "+c" (size) : "d" (port) );
}
