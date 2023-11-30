package stringid // import "github.com/AntiBargu/lkp-extent/lkp-master/pkg/stringid"

import (
	"crypto/rand"
	"encoding/hex"
)

// GenerateRandomID returns a unique id.
func GenerateRandomID() string {
	b := make([]byte, 32)

	if _, err := rand.Read(b); err != nil {
		panic(err) // This shouldn't happen
	}

	return hex.EncodeToString(b)
}
