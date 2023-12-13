package informer // import "github.com/AntiBargu/lkp-extent/lkp-node/informer"

import (
	"bufio"
	"fmt"
	"os"
	"strings"
	"syscall"
)

func KernelVersion() string {
	file, err := os.Open("/proc/version")
	if err != nil {
		fmt.Fprintf(os.Stderr, "%s\n", err.Error())
		return ""
	}
	defer file.Close()

	scanner := bufio.NewScanner(file)
	scanner.Scan()
	line := scanner.Text()

	fields := strings.Fields(line)
	return fields[2]
}

func Distribution() string {
	file, err := os.Open("/etc/os-release")
	if err != nil {
		fmt.Fprintf(os.Stderr, "%s\n", err.Error())
		return ""
	}
	defer file.Close()

	scanner := bufio.NewScanner(file)
	for scanner.Scan() {
		line := scanner.Text()
		if strings.HasPrefix(line, "PRETTY_NAME=") {
			return strings.Trim(line[len("PRETTY_NAME="):], "\"")
		}
	}

	return ""
}

func CPUInfo() string {
	file, err := os.Open("/proc/cpuinfo")
	if err != nil {
		fmt.Fprintf(os.Stderr, "%s\n", err.Error())
		return ""
	}
	defer file.Close()

	scanner := bufio.NewScanner(file)
	for scanner.Scan() {
		line := scanner.Text()
		if strings.HasPrefix(line, "model name") {
			return strings.Trim(line[len("model name")+2:], " \t\n")
		}
	}

	return ""
}

func MemorySize() uint64 {
	var info syscall.Sysinfo_t
	if err := syscall.Sysinfo(&info); err != nil {
		fmt.Fprintf(os.Stderr, "%s\n", err.Error())
		return 0
	}

	return info.Totalram * uint64(info.Unit)
}
