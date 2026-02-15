# Test Makefile for pho
# Uses Unity testing framework

CC = cc
CFLAGS = -g -Wall -DVERSION='"test"' -I.. -I../exif -Iunity \
         $(shell pkg-config --cflags gtk+-3.0 2>/dev/null)

LDFLAGS = $(shell pkg-config --libs gtk+-3.0 2>/dev/null) -lm

# Unity framework
UNITY_SRC = unity/unity.c
UNITY_OBJ = unity/unity.o

# Test executables
UNIT_TESTS = unit/test_pho unit/test_phoimglist
REGRESSION_TESTS = regression/test_issue_1 regression/test_issue_2 regression/test_issue_3 regression/test_issue_4 regression/test_issue_5 regression/test_issue_6 regression/test_issues_7_12 regression/test_issues_14_21 regression/test_features_raw_and_slideshow
ALL_TESTS = $(UNIT_TESTS) $(REGRESSION_TESTS)

# Default target: build all tests
.PHONY: all clean test

all: $(ALL_TESTS)

# Unity object file
$(UNITY_OBJ): $(UNITY_SRC)
	$(CC) $(CFLAGS) -c $< -o $@

# Source files needed for tests (excluding gmain.c which has main())
PHO_SRCS = ../pho.c ../phoimglist.c ../imagenote.c ../gdialogs.c ../keydialog.c ../gwin.c test_stub.c

# Pattern rule for test executables
unit/test_pho: unit/test_pho.c $(UNITY_OBJ) $(PHO_SRCS) ../exif/libphoexif.a
	$(CC) $(CFLAGS) -o $@ unit/test_pho.c $(UNITY_OBJ) $(PHO_SRCS) ../exif/libphoexif.a $(LDFLAGS)

unit/test_phoimglist: unit/test_phoimglist.c $(UNITY_OBJ) $(PHO_SRCS) ../exif/libphoexif.a
	$(CC) $(CFLAGS) -o $@ unit/test_phoimglist.c $(UNITY_OBJ) $(PHO_SRCS) ../exif/libphoexif.a $(LDFLAGS)

regression/test_issue_1: regression/test_issue_1.c $(UNITY_OBJ) $(PHO_SRCS) ../exif/libphoexif.a
	$(CC) $(CFLAGS) -o $@ regression/test_issue_1.c $(UNITY_OBJ) $(PHO_SRCS) ../exif/libphoexif.a $(LDFLAGS)

regression/test_issue_2: regression/test_issue_2.c $(UNITY_OBJ) $(PHO_SRCS) ../exif/libphoexif.a
	$(CC) $(CFLAGS) -o $@ regression/test_issue_2.c $(UNITY_OBJ) $(PHO_SRCS) ../exif/libphoexif.a $(LDFLAGS)

regression/test_issue_3: regression/test_issue_3.c $(UNITY_OBJ) $(PHO_SRCS) ../exif/libphoexif.a
	$(CC) $(CFLAGS) -o $@ regression/test_issue_3.c $(UNITY_OBJ) $(PHO_SRCS) ../exif/libphoexif.a $(LDFLAGS)

regression/test_issue_4: regression/test_issue_4.c $(UNITY_OBJ) $(PHO_SRCS) ../exif/libphoexif.a
	$(CC) $(CFLAGS) -o $@ regression/test_issue_4.c $(UNITY_OBJ) $(PHO_SRCS) ../exif/libphoexif.a $(LDFLAGS)

regression/test_issue_5: regression/test_issue_5.c $(UNITY_OBJ) $(PHO_SRCS) ../exif/libphoexif.a
	$(CC) $(CFLAGS) -o $@ regression/test_issue_5.c $(UNITY_OBJ) $(PHO_SRCS) ../exif/libphoexif.a $(LDFLAGS)

regression/test_issue_6: regression/test_issue_6.c $(UNITY_OBJ) $(PHO_SRCS) ../exif/libphoexif.a
	$(CC) $(CFLAGS) -o $@ regression/test_issue_6.c $(UNITY_OBJ) $(PHO_SRCS) ../exif/libphoexif.a $(LDFLAGS)

regression/test_issues_7_12: regression/test_issues_7_12.c $(UNITY_OBJ) $(PHO_SRCS) ../exif/libphoexif.a
	$(CC) $(CFLAGS) -o $@ regression/test_issues_7_12.c $(UNITY_OBJ) $(PHO_SRCS) ../exif/libphoexif.a $(LDFLAGS)

regression/test_issues_14_21: regression/test_issues_14_21.c $(UNITY_OBJ) $(PHO_SRCS) ../exif/libphoexif.a
	$(CC) $(CFLAGS) -o $@ regression/test_issues_14_21.c $(UNITY_OBJ) $(PHO_SRCS) ../exif/libphoexif.a $(LDFLAGS)

regression/test_features_raw_and_slideshow: regression/test_features_raw_and_slideshow.c $(UNITY_OBJ) $(PHO_SRCS) ../exif/libphoexif.a
	$(CC) $(CFLAGS) -o $@ regression/test_features_raw_and_slideshow.c $(UNITY_OBJ) $(PHO_SRCS) ../exif/libphoexif.a $(LDFLAGS)

# Run all tests
test: all
	@echo "========================================"
	@echo "Running Unit Tests"
	@echo "========================================"
	@for test in $(UNIT_TESTS); do \
		echo "\n--- Running $$test ---"; \
		./$$test || exit 1; \
	done
	@echo "\n========================================"
	@echo "Running Regression Tests"
	@echo "========================================"
	@for test in $(REGRESSION_TESTS); do \
		echo "\n--- Running $$test ---"; \
		./$$test || exit 1; \
	done
	@echo "\n========================================"
	@echo "All tests passed!"
	@echo "========================================"

# Run only unit tests
test-unit: $(UNIT_TESTS)
	@echo "========================================"
	@echo "Running Unit Tests"
	@echo "========================================"
	@for test in $(UNIT_TESTS); do \
		echo "\n--- Running $$test ---"; \
		./$$test || exit 1; \
	done

# Run only regression tests
test-regression: $(REGRESSION_TESTS)
	@echo "========================================"
	@echo "Running Regression Tests"
	@echo "========================================"
	@for test in $(REGRESSION_TESTS); do \
		echo "\n--- Running $$test ---"; \
		./$$test || exit 1; \
	done

# Clean test artifacts
clean:
	rm -f $(UNITY_OBJ) $(ALL_TESTS)

# Help
help:
	@echo "Test targets:"
	@echo "  make all            - Build all tests"
	@echo "  make test           - Build and run all tests"
	@echo "  make test-unit      - Run unit tests only"
	@echo "  make test-regression- Run regression tests only"
	@echo "  make clean          - Remove test artifacts"
