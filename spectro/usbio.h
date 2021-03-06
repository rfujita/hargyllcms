
#ifndef USBIO_H

/* 
 * Argyll Color Correction System
 *
 * General USB I/O support
 *
 * Author: Graeme W. Gill
 * Date:   2006/22/4
 *
 * Copyright 2006 - 2007 Graeme W. Gill
 * All rights reserved.
 *
 * This material is licenced under the GNU AFFERO GENERAL PUBLIC LICENSE Version 3 :-
 * see the License.txt file for licencing details.
 */

/* These routines supliement the class code in ntio.c and unixio.c */
/* They do benign things if ENABLE_USB is undefined */

/* Add paths to USB connected instruments, to the existing */
/* icompath paths in the icoms structure. */
void usb_get_paths(
struct _icoms *p 
);

/* Return the instrument type if the port number is USB, */
/* and instUnknown if it is not. */
instType usb_is_usb_portno(icoms *p, int port);

void usb_close_port(icoms *p);

/* Set the USB specific icoms methods */
void usb_set_usb_methods(icoms *p);

/* Install the cleanup signal handlers */
/* (used inside usb_open_port(), hid_open_port() */
void usb_install_signal_handlers(icoms *p);

/* Delete an icoms from our static signal cleanup list */
/* (used inside usb_close_port(), hid_close_port() */
void usb_delete_from_cleanup_list(icoms *p);

/* Cleanup and then free a usb dev entry */
void usb_del_usb_device(struct usb_device *dev);

#define USBIO_H
#endif /* USBIO_H */
