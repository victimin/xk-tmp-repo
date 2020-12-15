#include "XK_RadarApplicationInfo.h"


app_info_t gAppInfo[APP_SIZE] = {
	{
		.appNum = 40,
		.appName = "presence",
		.appSerialNumIdx = 20,
		.appParamSZ = 2,
		.param[0] = {
			.flgSIC	= 0,
			.paramObjNum_V = 11,
			.paramName = "presence"
		},
		.param[1] = {
			.flgSIC	= 0,
			.paramObjNum_V = 15,
			.paramName = "version"
		}
	},
	{
		.appNum = 41,
		.appName = "presence_vital",
		.appSerialNumIdx = 20,
		.appParamSZ = 8,
		.param[0] = {
			.flgSIC	= 0,
			.paramObjNum_V = 11,
			.paramName = "presence"
		},
		.param[1] = {
			.flgSIC	= 0,
			.paramObjNum_V = 26,
			.paramName = "BR"
		},
		.param[2] = {
			.flgSIC	= 0,
			.paramObjNum_V = 27,
			.paramName = "HR"
		},
		.param[3] = {
			.flgSIC	= 0,
			.paramObjNum_V = 16,
			.paramName = "movement_index"
		},
		.param[4] = {
			.flgSIC	= 0,
			.paramObjNum_V = 24,
			.paramName = "stability_BR"
		},
		.param[5] = {
			.flgSIC	= 0,
			.paramObjNum_V = 25,
			.paramName = "stability_HR"
		},
		.param[6] = {
			.flgSIC	= 0,
			.paramObjNum_V = 28,
			.paramName = "stability_move"
		},
		.param[7] = {
			.flgSIC	= 0,
			.paramObjNum_V = 15,
			.paramName = "version"
		}
	},
	{
		.appNum = 30,
		.appName = "zone",
		.appSerialNumIdx = 27,
		.appParamSZ = 8,
		.param[0] = {
			.flgSIC	= 0,
			.paramObjNum_V = 4,
			.paramName = "presence"
		},
		.param[1] = {
			.flgSIC	= 0,
			.paramObjNum_V = 24,
			.paramName = "peoplecount_instant"
		},
		.param[2] = {
			.flgSIC	= 0,
			.paramObjNum_V = 15,
			.paramName = "peoplecount_average"
		},
		.param[3] = {
			.flgSIC	= 0,
			.paramObjNum_V = 7,
			.paramName = "index_longterm"
		},
		.param[4] = {
			.flgSIC	= 0,
			.paramObjNum_V = 6,
			.paramName = "index_shortterm"
		},
		.param[5] = {
			.flgSIC	= 0,
			.paramObjNum_V = 5,
			.paramName = "version"
		},
		.param[6] = {
			.flgSIC	= 0,
			.paramObjNum_V = 25,
			.paramName = "radarmappingvalue"
		},
		.param[7] = {
			.flgSIC	= 0,
			.paramObjNum_V = 26,
			.paramName = "environmappingvalue"
		}
	},
	{
		.appNum = 31,
		.appName = "zone_combo",
		.appSerialNumIdx = 27,
		.appParamSZ = 11,
		.param[0] = {
			.flgSIC	= 0,
			.paramObjNum_V = 4,
			.paramName = "presence"
		},
		.param[1] = {
			.flgSIC	= 0,
			.paramObjNum_V = 24,
			.paramName = "peoplecount_instant"
		},
		.param[2] = {
			.flgSIC	= 0,
			.paramObjNum_V = 15,
			.paramName = "peoplecount_average"
		},
		.param[3] = {
			.flgSIC	= 0,
			.paramObjNum_V = 7,
			.paramName = "index_longterm"
		},
		.param[4] = {
			.flgSIC	= 0,
			.paramObjNum_V = 6,
			.paramName = "index_shortterm"
		},
		.param[5] = {
			.flgSIC	= 0,
			.paramObjNum_V = 5,
			.paramName = "version"
		},
		.param[6] = {
			.flgSIC	= 0,
			.paramObjNum_V = 25,
			.paramName = "radarmappingvalue"
		},
		.param[7] = {
			.flgSIC	= 0,
			.paramObjNum_V = 26,
			.paramName = "environmappingvalue"
		},
		.param[8] = {
			.flgSIC	= 0,
			.paramObjNum_V = 44,
			.paramName = "counting_result"
		},
		.param[9] = {
			.flgSIC	= 0,
			.paramObjNum_V = 47,
			.paramName = "number_zone_sensors"
		},
		.param[10] = {
			.flgSIC	= 0,
			.paramObjNum_V = 48,
			.paramName = "number_inout_sensors"
		}
	},
	{
		.appNum = 80,
		.appName = "vital",
		.appSerialNumIdx = 16,
		.appParamSZ = 8,
		.param[0] = {
			.flgSIC	= 0,
			.paramObjNum_V = 4,
			.paramName = "BR"
		},
		.param[1] = {
			.flgSIC	= 0,
			.paramObjNum_V = 5,
			.paramName = "HR"
		},
		.param[2] = {
			.flgSIC	= 0,
			.paramObjNum_V = 3,
			.paramName = "movement_index"
		},
		.param[3] = {
			.flgSIC	= 0,
			.paramObjNum_V = 6,
			.paramName = "stability_BR"
		},
		.param[4] = {
			.flgSIC	= 0,
			.paramObjNum_V = 7,
			.paramName = "stability_HR"
		},
		.param[5] = {
			.flgSIC	= 0,
			.paramObjNum_V = 23,
			.paramName = "stability_move"
		},
		.param[6] = {
			.flgSIC	= 0,
			.paramObjNum_V = 2,
			.paramName = "connectionstatus"
		},
		.param[7] = {
			.flgSIC	= 0,
			.paramObjNum_V = 15,
			.paramName = "version"
		}
	},
	{
		.appNum = 60,
		.appName = "foot",
		.appSerialNumIdx = 19,
		.appParamSZ = 5,
		.param[0] = {
			.flgSIC	= 0,
			.paramObjNum_V = 4,
			.paramName = "flowcount_instant"
		},
		.param[1] = {
			.flgSIC	= 0,
			.paramObjNum_V = 11,
			.paramName = "flowcount_accumulated"
		},
		.param[2] = {
			.flgSIC	= 0,
			.paramObjNum_V = 17,
			.paramName = "updatecounter"
		},
		.param[3] = {
			.flgSIC	= 0,
			.paramObjNum_V = 6,
			.paramName = "connectionstatus"
		},
		.param[4] = {
			.flgSIC	= 0,
			.paramObjNum_V = 7,
			.paramName = "version"
		}
	},
	{
		.appNum = 70,
		.appName = "OSR",
		.appSerialNumIdx = 20,
		.appParamSZ = 3,
		.param[0] = {
			.flgSIC	= 0,
			.paramObjNum_V = 11,
			.paramName = "presence"
		},
		.param[1] = {
			.flgSIC	= 0,
			.paramObjNum_V = 10,
			.paramName = "OSR_result"
		},
		.param[2] = {
			.flgSIC	= 0,
			.paramObjNum_V = 15,
			.paramName = "version"
		}
	},
	{
		.appNum = 71,
		.appName = "Skimmer",
		.appSerialNumIdx = 20,
		.appParamSZ = 3,
		.param[0] = {
			.flgSIC	= 0,
			.paramObjNum_V = 11,
			.paramName = "presence"
		},
		.param[1] = {
			.flgSIC	= 0,
			.paramObjNum_V = 10,
			.paramName = "OSR_result"
		},
		.param[2] = {
			.flgSIC	= 0,
			.paramObjNum_V = 15,
			.paramName = "version"
		}
	},
	{
		.appNum = 21,
		.appName = "INOUT_single",
		.appSerialNumIdx = 26,
		.appParamSZ = 6,
		.param[0] = {
			.flgSIC	= 0,
			.paramObjNum_V = 6,
			.paramName = "connectionstatus"
		},
		.param[1] = {
			.flgSIC	= 0,
			.paramObjNum_V = 14,
			.paramName = "INcount"
		},
		.param[2] = {
			.flgSIC	= 0,
			.paramObjNum_V = 15,
			.paramName = "OUTcount"
		},
		.param[3] = {
			.flgSIC	= 0,
			.paramObjNum_V = 16,
			.paramName = "INcount_accumulated"
		},
		.param[4] = {
			.flgSIC	= 0,
			.paramObjNum_V = 17,
			.paramName = "OUTcount_accumulated"
		},
		.param[5] = {
			.flgSIC	= 0,
			.paramObjNum_V = 22,
			.paramName = "updatecounter"
		}
	},
	{
		.appNum = 50,
		.appName = "WMFD",
		.appSerialNumIdx = 20,
		.appParamSZ = 8,
		.param[0] = {
			.flgSIC	= 0,
			.paramObjNum_V = 6,
			.paramName = "presence"
		},
		.param[1] = {
			.flgSIC	= 0,
			.paramObjNum_V = 0,
			.paramName = "fall_status"
		},
		.param[2] = {
			.flgSIC	= 0,
			.paramObjNum_V = 1,
			.paramName = "BR"
		},
		.param[3] = {
			.flgSIC	= 0,
			.paramObjNum_V = 3,
			.paramName = "HR"
		},
		.param[4] = {
			.flgSIC	= 0,
			.paramObjNum_V = 5,
			.paramName = "movement_index"
		},
		.param[5] = {
			.flgSIC	= 0,
			.paramObjNum_V = 2,
			.paramName = "stability_BR"
		},
		.param[6] = {
			.flgSIC	= 0,
			.paramObjNum_V = 4,
			.paramName = "stability_HR"
		},
		.param[7] = {
			.flgSIC	= 0,
			.paramObjNum_V = 20,
			.paramName = "stability_move"
		}
	}
};

