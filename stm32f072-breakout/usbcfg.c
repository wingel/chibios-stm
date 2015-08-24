/*
    ChibiOS/RT - Copyright (C) 2006-2013 Giovanni Di Sirio

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

#include "ch.h"
#include "hal.h"

#include "usbcfg.h"

/* Index into string descriptor table */
enum
{
    iLanguage,
    iManufacturer,
    iProduct,
    iSerial,
    iAcm,
    iAdc,
};

/*
 * USB Device Descriptor.
 */
static const uint8_t vcom_device_descriptor_data[18] = {
  USB_DESC_DEVICE       (0x0110,        /* bcdUSB (1.1).                    */
                         0xef,          /* bDeviceClass (Misc Device).      */
                         0x02,          /* bDeviceSubClass (Common).        */
                         0x02,          /* bDeviceProtocol (IAD).           */
                         64,            /* bMaxPacketSize.                  */
                         0x0483,        /* idVendor (ST).                   */
                         0x5740,        /* idProduct.                       */
                         0x0200,        /* bcdDevice.                       */
                         iManufacturer, /* iManufacturer.                   */
                         iProduct,      /* iProduct.                        */
                         iSerial,       /* iSerialNumber.                   */
                         1)             /* bNumConfigurations.              */
};

/*
 * Device Descriptor wrapper.
 */
static const USBDescriptor vcom_device_descriptor = {
  sizeof vcom_device_descriptor_data,
  vcom_device_descriptor_data
};

/* Configuration Descriptor tree for a CDC.*/
static const uint8_t vcom_configuration_descriptor_data[99] = {
  /* Configuration Descriptor.*/
    USB_DESC_CONFIGURATION(102,         /* wTotalLength.                    */
                         0x03,          /* bNumInterfaces.                  */
                         0x01,          /* bConfigurationValue.             */
                         0,             /* iConfiguration.                  */
                         0xC0,          /* bmAttributes (self powered).     */
                         50),           /* bMaxPower (100mA).               */
  /* Interface Association Descriptor.*/
  USB_DESC_INTERFACE_ASSOCIATION(0x00, /* bFirstInterface.                  */
                                 0x02, /* bInterfaceCount.                  */
                                 0x02, /* bFunctionClass (CDC).             */
                                 0x00, /* bFunctionSubClass.                */
                                 0x00, /* bFunctionProcotol                 */
                                 iAcm), /* iInterface.               */
  /* Interface Descriptor.*/
  USB_DESC_INTERFACE    (0x00,          /* bInterfaceNumber.                */
                         0x00,          /* bAlternateSetting.               */
                         0x01,          /* bNumEndpoints.                   */
                         0x02,          /* bInterfaceClass (Communications
                                           Interface Class, CDC section
                                           4.2).                            */
                         0x02,          /* bInterfaceSubClass (Abstract
                                         Control Model, CDC section 4.3).   */
                         0x01,          /* bInterfaceProtocol (AT commands,
                                           CDC section 4.4).                */
                         iAcm),  /* iInterface.                      */
  /* Header Functional Descriptor (CDC section 5.2.3).*/
  USB_DESC_BYTE         (5),            /* bLength.                         */
  USB_DESC_BYTE         (0x24),         /* bDescriptorType (CS_INTERFACE).  */
  USB_DESC_BYTE         (0x00),         /* bDescriptorSubtype (Header
                                           Functional Descriptor.           */
  USB_DESC_BCD          (0x0110),       /* bcdCDC.                          */
  /* Call Management Functional Descriptor. */
  USB_DESC_BYTE         (5),            /* bFunctionLength.                 */
  USB_DESC_BYTE         (0x24),         /* bDescriptorType (CS_INTERFACE).  */
  USB_DESC_BYTE         (0x01),         /* bDescriptorSubtype (Call Management
                                           Functional Descriptor).          */
  USB_DESC_BYTE         (0x00),         /* bmCapabilities (D0+D1).          */
  USB_DESC_BYTE         (0x01),         /* bDataInterface.                  */
  /* ACM Functional Descriptor.*/
  USB_DESC_BYTE         (4),            /* bFunctionLength.                 */
  USB_DESC_BYTE         (0x24),         /* bDescriptorType (CS_INTERFACE).  */
  USB_DESC_BYTE         (0x02),         /* bDescriptorSubtype (Abstract
                                           Control Management Descriptor).  */
  USB_DESC_BYTE         (0x02),         /* bmCapabilities.                  */
  /* Union Functional Descriptor.*/
  USB_DESC_BYTE         (5),            /* bFunctionLength.                 */
  USB_DESC_BYTE         (0x24),         /* bDescriptorType (CS_INTERFACE).  */
  USB_DESC_BYTE         (0x06),         /* bDescriptorSubtype (Union
                                           Functional Descriptor).          */
  USB_DESC_BYTE         (0x00),         /* bMasterInterface (Communication
                                           Class Interface).                */
  USB_DESC_BYTE         (0x01),         /* bSlaveInterface0 (Data Class
                                           Interface).                      */
  /* Endpoint 2 Descriptor.*/
  USB_DESC_ENDPOINT     (USBD1_SDU_INTERRUPT_EP|0x80,
                         0x03,          /* bmAttributes (Interrupt).        */
                         0x0008,        /* wMaxPacketSize.                  */
                         0xFF),         /* bInterval.                       */
  /* Interface Descriptor.*/
  USB_DESC_INTERFACE    (0x01,          /* bInterfaceNumber.                */
                         0x00,          /* bAlternateSetting.               */
                         0x02,          /* bNumEndpoints.                   */
                         0x0A,          /* bInterfaceClass (Data Class
                                           Interface, CDC section 4.5).     */
                         0x00,          /* bInterfaceSubClass (CDC section
                                           4.6).                            */
                         0x00,          /* bInterfaceProtocol (CDC section
                                           4.7).                            */
                         iAcm),  /* iInterface.                      */
  /* Endpoint 1 Descriptor.*/
  USB_DESC_ENDPOINT     (USBD1_SDU_DATA_EP,       /* bEndpointAddress.*/
                         0x02,          /* bmAttributes (Bulk).             */
                         64,            /* wMaxPacketSize.                  */
                         0x00),         /* bInterval.                       */
  /* Endpoint 1 Descriptor.*/
  USB_DESC_ENDPOINT     (USBD1_SDU_DATA_EP|0x80,    /* bEndpointAddress.*/
                         0x02,          /* bmAttributes (Bulk).             */
                         64,            /* wMaxPacketSize.                  */
                         0x00),         /* bInterval.                       */

  /* Interface Association Descriptor.*/
  USB_DESC_INTERFACE_ASSOCIATION(0x02, /* bFirstInterface.                  */
                              0x01, /* bInterfaceCount.                  */
                              0xFF, /* bFunctionClass (Vendor Specific).  */
                              0x00, /* bFunctionSubClass.                */
                              0x00, /* bFunctionProcotol                 */
                              iAdc),   /* iInterface.                       */
  /* Interface Descriptor.*/
  USB_DESC_INTERFACE    (0x02,          /* bInterfaceNumber.                */
                         0x00,          /* bAlternateSetting.               */
                         0x01,          /* bNumEndpoints.                   */
                         0xFF,          /* bInterfaceClass (Vendor Specific). */
                         0x00,
                         0x00,
                         iAdc),         /* iInterface.                      */
  /* Endpoint 3 Descriptor.*/
  USB_DESC_ENDPOINT     (USBD1_ADC_DATA_EP |0x80,      /* bEndpointAddress.*/
                         0x02,          /* bmAttributes (Bulk).             */
                           64,          /* wMaxPacketSize.                  */
                         0x00),         /* bInterval.                       */
};

/*
 * Configuration Descriptor wrapper.
 */
static const USBDescriptor vcom_configuration_descriptor = {
  sizeof vcom_configuration_descriptor_data,
  vcom_configuration_descriptor_data
};

/*
 * U.S. English language identifier.
 */
static const uint8_t sLanguage[] = {
  USB_DESC_BYTE(4),                     /* bLength.                         */
  USB_DESC_BYTE(USB_DESCRIPTOR_STRING), /* bDescriptorType.                 */
  USB_DESC_WORD(0x0409)                 /* wLANGID (U.S. English).          */
};

/*
 * Vendor string.
 */
static const uint8_t sManufacturer[] = {
  USB_DESC_BYTE(38),                    /* bLength.                         */
  USB_DESC_BYTE(USB_DESCRIPTOR_STRING), /* bDescriptorType.                 */
  'S', 0, 'T', 0, 'M', 0, 'i', 0, 'c', 0, 'r', 0, 'o', 0, 'e', 0,
  'l', 0, 'e', 0, 'c', 0, 't', 0, 'r', 0, 'o', 0, 'n', 0, 'i', 0,
  'c', 0, 's', 0
};

/*
 * Device Description string.
 */
static const uint8_t sProduct[] = {
  USB_DESC_BYTE(12),                    /* bLength.                         */
  USB_DESC_BYTE(USB_DESCRIPTOR_STRING), /* bDescriptorType.                 */
  'S', 0, 'T', 0, 'M', 0, '3', 0, '2', 0
};

/*
 * Device Description string.
 */
static const uint8_t sAcm[] = {
  USB_DESC_BYTE(56),                    /* bLength.                         */
  USB_DESC_BYTE(USB_DESCRIPTOR_STRING), /* bDescriptorType.                 */
  'C', 0, 'h', 0, 'i', 0, 'b', 0, 'i', 0, 'O', 0, 'S', 0, '/', 0,
  'R', 0, 'T', 0, ' ', 0, 'V', 0, 'i', 0, 'r', 0, 't', 0, 'u', 0,
  'a', 0, 'l', 0, ' ', 0, 'C', 0, 'O', 0, 'M', 0, ' ', 0, 'P', 0,
  'o', 0, 'r', 0, 't', 0
};

/*
 * Device Description string.
 */
static const uint8_t sAdc[] = {
  USB_DESC_BYTE(18),                    /* bLength.                         */
  USB_DESC_BYTE(USB_DESCRIPTOR_STRING), /* bDescriptorType.                 */
  'A', 0, 'D', 0, 'C', 0, ' ', 0, 'D', 0, 'a', 0, 't', 0, 'a', 0
};

/*
 * Serial Number string.
 */
uint8_t vcom_serial[] = {
  USB_DESC_BYTE(2 + 48),                /* bLength.                         */
  USB_DESC_BYTE(USB_DESCRIPTOR_STRING), /* bDescriptorType.                 */
  '0', 0, '0', 0, '0', 0, '0', 0, '0', 0, '0', 0, '0', 0, '0', 0,
  '0', 0, '0', 0, '0', 0, '0', 0, '0', 0, '0', 0, '0', 0, '0', 0,
  '0', 0, '0', 0, '0', 0, '0', 0, '0', 0, '0', 0, '0', 0, '0', 0,
};

/*
 * Strings wrappers array.
 */
static const USBDescriptor vcom_strings[] = {
    [ iLanguage     ] = { sizeof(sLanguage),    sLanguage     },
    [ iManufacturer ] = { sizeof sManufacturer, sManufacturer },
    [ iProduct      ] = { sizeof sProduct,      sProduct      },
    [ iSerial       ] = { sizeof vcom_serial,   vcom_serial   },
    [ iAcm          ] = { sizeof sAcm,          sAcm          },
    [ iAdc          ] = { sizeof sAdc,          sAdc          },
};

/*
 * Handles the GET_DESCRIPTOR callback. All required descriptors must be
 * handled here.
 */
static const USBDescriptor *get_descriptor(USBDriver *usbp,
                                           uint8_t dtype,
                                           uint8_t dindex,
                                           uint16_t lang) {

  (void)usbp;
  (void)lang;
  switch (dtype) {
  case USB_DESCRIPTOR_DEVICE:
    return &vcom_device_descriptor;
  case USB_DESCRIPTOR_CONFIGURATION:
    return &vcom_configuration_descriptor;
  case USB_DESCRIPTOR_STRING:
      if (dindex < sizeof(vcom_strings) / sizeof(*vcom_strings))
      {
	  if (vcom_strings[dindex].ud_string)
	      return &vcom_strings[dindex];
      }
  }
  return NULL;
}

/**
 * @brief   SDU data IN endpoint state.
 */
static USBInEndpointState sduDataInState;

/**
 * @brief   SDU data out endpoint state.
 */
static USBOutEndpointState sduDataOutState;

/**
 * @brief   EP1 initialization structure (both IN and OUT).
 */
static const USBEndpointConfig sduDataEpConfig = {
  USB_EP_MODE_TYPE_BULK,
  NULL,
  sduDataTransmitted,
  sduDataReceived,
  64,
  64,
  &sduDataInState,
  &sduDataOutState,
  2,
  NULL
};

/**
 * @brief   SDI interrupt IN endpoint state.
 */
static USBInEndpointState sduInterruptInState;

/**
 * @brief   EP2 initialization structure (IN only).
 */
static const USBEndpointConfig sduInterruptEpConfig = {
  USB_EP_MODE_TYPE_INTR,
  NULL,
  sduInterruptTransmitted,
  NULL,
  16,
  0,
  &sduInterruptInState,
  NULL,
  1,
  NULL
};

#if 0
/**
 * @brief   IN EP2 state.
 */
static USBInEndpointState ep2instate;

/**
 * @brief   ADC Endpoint initialization structure (OUT only).
 */
static const USBEndpointConfig adcDataEpConfig = {
  USB_EP_MODE_TYPE_BULK,
  NULL,
  adcDataTransmitted,
  NULL,
  64,
  0,
  &ep2instate,
  NULL,
  1,
  NULL
};
#endif

/*
 * Handles the USB driver global events.
 */
static void usb_event(USBDriver *usbp, usbevent_t event) {
  extern SerialUSBDriver SDU1;

  switch (event) {
  case USB_EVENT_RESET:
    return;
  case USB_EVENT_ADDRESS:
    return;
  case USB_EVENT_CONFIGURED:
    chSysLockFromISR();

    /* Enables the endpoints specified into the configuration.
       Note, this callback is invoked from an ISR so I-Class functions
       must be used.*/
    usbInitEndpointI(usbp, USBD1_SDU_DATA_EP, &sduDataEpConfig);
    usbInitEndpointI(usbp, USBD1_SDU_INTERRUPT_EP, &sduInterruptEpConfig);
    // usbInitEndpointI(usbp, USBD1_ADC_DATA_EP, &adcDataEpConfig);

    /* Resetting the state of the CDC subsystem.*/
    sduConfigureHookI(&SDU1);

    chSysUnlockFromISR();
    return;
  case USB_EVENT_SUSPEND:
    return;
  case USB_EVENT_WAKEUP:
    return;
  case USB_EVENT_STALLED:
    return;
  }
  return;
}

/*
 * USB driver configuration.
 */
const USBConfig usbcfg = {
  usb_event,
  get_descriptor,
  sduRequestsHook,
  NULL
};

/*
 * Serial over USB driver configuration.
 */
const SerialUSBConfig serusbcfg = {
  &USBD1,
  USBD1_SDU_DATA_EP,
  USBD1_SDU_DATA_EP,
  USBD1_SDU_INTERRUPT_EP
};

//USBDriver *const adcUsbp = &USBD1;
//const usbep_t adcDataEp = USBD1_ADC_DATA_EP;
