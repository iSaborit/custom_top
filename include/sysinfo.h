//
// Created by Iker Saborit López on 15/2/26.
//


#ifndef CUSTOM_TOP_SYSINFO_H
#define CUSTOM_TOP_SYSINFO_H

#include "process.h"

/* I can use
 * proc_listpids
 * proc_name
 * proc_pidinfo
*/

ProcessArray *get_process_list(void);

#endif //CUSTOM_TOP_SYSINFO_H
