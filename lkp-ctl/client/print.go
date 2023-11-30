package client // import "github.com/AntiBargu/lkp-extent/lkp-ctl/client"

type gracePrintCfg struct {
	discription string
	fmt         string
	show        bool
}

// Graceful print default config
var nodeListFmt = []gracePrintCfg{
	{
		discription: "NODE ID",
		fmt:         "%-18s",
		show:        true,
	}, // 0

	{
		discription: "IP",
		fmt:         "%-15s",
		show:        true,
	}, // 1

	{
		discription: "PORT",
		fmt:         "%-7s",
		show:        true,
	}, // 2

	{
		discription: "KERNEL VERSION",
		fmt:         "%-30s",
		show:        true,
	}, // 3

	{
		discription: "DISTRIBUTION",
		fmt:         "%-25s",
		show:        true,
	}, // 4

	{
		discription: "ARCH",
		fmt:         "%-50s",
		show:        true,
	}, // 5

	{
		discription: "MEM",
		fmt:         "%-10s",
		show:        true,
	}, // 6

	{
		discription: "LATENCY",
		fmt:         "%-7s",
		show:        false,
	}, // 7
}

var caseListFmt = []gracePrintCfg{
	{
		discription: "CASE NAME",
		fmt:         "%-40s",
		show:        true,
	}, // 0

	{
		discription: "TAG",
		fmt:         "%-10s",
		show:        true,
	}, // 1

	{
		discription: "CASE ID",
		fmt:         "%-18s",
		show:        true,
	}, // 2

	{
		discription: "CREATE TIME",
		fmt:         "%-25s",
		show:        true,
	}, // 3

	{
		discription: "ACCESS TIME",
		fmt:         "%-25s",
		show:        true,
	}, // 4

	{
		discription: "MODIFY TIME",
		fmt:         "%-25s",
		show:        true,
	}, // 5

	{
		discription: "SIZE",
		fmt:         "%-5s",
		show:        true,
	}, // 5
}

var jobListFmt = []gracePrintCfg{
	{
		discription: "JOB ID",
		fmt:         "%-18s",
		show:        true,
	}, // 0

	{
		discription: "JOB NAME",
		fmt:         "%-25s",
		show:        true,
	}, // 1

	{
		discription: "CASE ID",
		fmt:         "%-18s",
		show:        true,
	}, // 2

	{
		discription: "CREATE TIME",
		fmt:         "%-25s",
		show:        true,
	}, // 3

	{
		discription: "FINISH TIME",
		fmt:         "%-25s",
		show:        true,
	}, // 4

	{
		discription: "STATUS",
		fmt:         "%-10s",
		show:        true,
	}, // 5

	{
		discription: "FINISH",
		fmt:         "%-10s",
		show:        true,
	}, // 6
}

var taskListFmt = []gracePrintCfg{
	{
		discription: "TASK ID",
		fmt:         "%-18s",
		show:        true,
	}, // 0

	{
		discription: "CASE ID",
		fmt:         "%-18s",
		show:        true,
	}, // 1

	{
		discription: "NODE ID",
		fmt:         "%-18s",
		show:        true,
	}, // 2

	{
		discription: "NODE IP",
		fmt:         "%-15s",
		show:        true,
	}, // 3

	{
		discription: "ROLE",
		fmt:         "%-15s",
		show:        true,
	}, // 4

	{
		discription: "CREATE TIME",
		fmt:         "%-25s",
		show:        true,
	}, // 5

	{
		discription: "FINISH TIME",
		fmt:         "%-25s",
		show:        true,
	}, // 6

	{
		discription: "STATUS",
		fmt:         "%-15s",
		show:        true,
	}, // 7
}
