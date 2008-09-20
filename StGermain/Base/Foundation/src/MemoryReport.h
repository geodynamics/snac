/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
**
** Copyright (C), 2003, Victorian Partnership for Advanced Computing (VPAC) Ltd, 110 Victoria Street, Melbourne, 3053 Australia.
**
** Authors:
**	Stevan M. Quenette, Senior Software Engineer, VPAC. (steve@vpac.org)
**	Patrick D. Sunter, Software Engineer, VPAC. (pds@vpac.org)
**	Luke J. Hodkinson, Computational Engineer, VPAC. (lhodkins@vpac.org)
**	Siew-Ching Tan, Software Engineer, VPAC. (siew@vpac.org)
**	Alan H. Lo, Computational Engineer, VPAC. (alan@vpac.org)
**	Raquibul Hassan, Computational Engineer, VPAC. (raq@vpac.org)
**
**  This library is free software; you can redistribute it and/or
**  modify it under the terms of the GNU Lesser General Public
**  License as published by the Free Software Foundation; either
**  version 2.1 of the License, or (at your option) any later version.
**
**  This library is distributed in the hope that it will be useful,
**  but WITHOUT ANY WARRANTY; without even the implied warranty of
**  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
**  Lesser General Public License for more details.
**
**  You should have received a copy of the GNU Lesser General Public
**  License along with this library; if not, write to the Free Software
**  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
**~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/** \file
** <b>Role:</b>
**	Used to construct a query and then print a report of the Memory Module.
**
** <b>Assumptions</b>
**	Memory Module is enabled.
**
** <b>Comments</b>
**	Do not use this class when the Memory Module is switched off.
**
** <b>Description</b>
**	To create custom queries of the data in the Memory Module, this class is used.
**	Note that the peak allocations statistic cannot be produced because it is a time-state dependant variable which cannot be
**	deduced from the existing data structure. Incorporating this information would otherwise impact the performance of the
**	module.
**
** $Id: MemoryReport.h 3462 2006-02-19 06:53:24Z WalterLandry $
**
**/

#ifndef __Base_Foundation_MemoryReport_h__
#define __Base_Foundation_MemoryReport_h__

	/** Textual name for MemoryReport class. */
	extern const Type MemoryReport_Type;

	/** Fields which in statistics which can be grouped or conditionally filtered */
	typedef enum {
		MEMORYREPORT_TYPE,
		MEMORYREPORT_NAME,
		MEMORYREPORT_FILE,
		MEMORYREPORT_FUNC
	} MemoryReportGroup;

	/** \def __MemoryReport See MemoryReport */
	#define __MemoryReport \
		Index			groupCount;		\
		Index			groupSize;		\
		MemoryReportGroup*	groups; 		/**< The groups to display in order. */ \
		Index			conditionCount;		\
		Index			conditionSize;		\
		MemoryReportGroup*	conditionGroups;	/**< The groups which are to be filtered by a condition. */ \
		char**			conditionValues;	/**< The value to match in the condition. */
	struct MemoryReport { __MemoryReport };

	/** Creates an empty report. */
	MemoryReport* MemoryReport_New();

	/** Initialises an empty report. */
	void _MemoryReport_Init( MemoryReport* memoryReport );
	
	/** Frees memory from a report. */
	void MemoryReport_Delete( MemoryReport* memoryReport );
	
	
	/** Adds a field to group by. Groups should be added in order of display. */
	void MemoryReport_AddGroup( MemoryReport* memoryReport, MemoryReportGroup group );
	
	/** Adds a condition where a field has to match a given value. If group does not exist, it will be automatically added. */
	void MemoryReport_AddCondition( MemoryReport* memoryReport, MemoryReportGroup group, const char* condition );
	

	/** Displays the report of the memory module based on the groups and conditions. */
	void MemoryReport_Print( MemoryReport* memoryReport );
	
	void MemoryReport_Print_Helper( void *memoryPointer, void* memoryReport );

		
#endif /* __Base_Foundation_MemoryReport_h__ */





