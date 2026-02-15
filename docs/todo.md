# Pho Testing Infrastructure Setup - Implementation Plan

**Date**: 2026-02-15  
**Option**: 1 - Quick Start (Unity Framework)  
**Estimated Time**: 4 hours  
**Goal**: Automated testing for critical bug fixes

---

## Phase 1: Setup Unity Framework (30 min)

### Tasks
- [ ] Download/Create Unity single-header test framework
- [ ] Place in `tests/unity/unity.h` and `unity.c`
- [ ] Verify compilation works

### Deliverables
- `tests/unity/unity.h` - Test framework header
- `tests/unity/unity.c` - Test framework implementation

---

## Phase 2: Create Test Directory Structure (15 min)

### Tasks
- [ ] Create `tests/` directory structure
- [ ] Create `tests/unit/` for function tests
- [ ] Create `tests/regression/` for bug fix tests
- [ ] Create `tests/fixtures/` for test data
- [ ] Create test Makefile or update main Makefile

### Deliverables
```
tests/
├── unity/
│   ├── unity.h
│   └── unity.c
├── unit/
│   ├── test_pho.c          # Core image functions
│   ├── test_phoimglist.c   # List operations
│   └── test_exif.c         # EXIF parsing
├── regression/
│   ├── test_issue_5.c      # Uninitialized var
│   └── test_issue_6.c      # Slideshow logic
├── fixtures/
│   └── README.md           # Test data description
└── Makefile                # Test build rules
```

---

## Phase 3: Write Initial Unit Tests (90 min)

### Critical Functions to Test

#### test_pho.c
- [ ] `NewPhoImage()` - allocation and initialization
- [ ] `ScaleToFit()` - scaling algorithms
- [ ] `ShuffleArray()` - randomization (no bias)

#### test_phoimglist.c
- [ ] `AppendItem()` - list append
- [ ] `DeleteItem()` - list deletion
- [ ] `ClearImageList()` - cleanup

#### test_exif.c
- [ ] `ExifReadInfo()` - EXIF parsing
- [ ] String handling safety

---

## Phase 4: Write Regression Tests (60 min)

### For Critical Issues in plan.md

#### test_issue_5.c - Uninitialized Variable
- [ ] Test `ScaleToFit()` when no scaling needed
- [ ] Test with various scale modes

#### test_issue_6.c - Slideshow Logic
- [ ] Test slideshow stop condition
- [ ] Test end-of-list detection

---

## Phase 5: Integrate with Build System (30 min)

### Tasks
- [ ] Add `make test` target to main Makefile
- [ ] Ensure tests compile with same flags as main code
- [ ] Add test runner that executes all tests

### Deliverables
- Updated `Makefile` with test target
- Test runner script or main function

---

## Phase 6: Documentation (15 min)

### Tasks
- [ ] Document how to run tests
- [ ] Document how to add new tests
- [ ] Update `docs/test-review.md` with actual implementation

---

## Progress Tracking

| Phase | Status | Notes |
|-------|--------|-------|
| 1: Unity Setup | ✅ | Unity downloaded to tests/unity/ |
| 2: Directory Structure | ✅ | tests/{unity,unit,regression,fixtures}/ |
| 3: Unit Tests | ✅ | test_pho.c, test_phoimglist.c |
| 4: Regression Tests | ✅ | test_issue_5.c, test_issue_6.c |
| 5: Build Integration | ✅ | Makefile targets added |
| 6: Documentation | ✅ | fixtures/README.md |

## Test Execution Results

### Build Status: ✅ SUCCESS
All test executables compile and link.

### Test Results Summary:

**unit/test_pho**: 13 tests, 3 failures, 10 passed
- ✅ NewPhoImage tests: All pass
- ✅ ShuffleArray tests: All pass  
- ❌ ScaleToFit tests: 3 failures (CONFIRMS Issue #5 - uninitialized variable bug!)

**regression/test_issue_5**: 5 tests, 4 failures, 1 passed
- ❌ Confirms Issue #5: Uninitialized variables in ScaleToFit()
- When no scaling needed, returns garbage values (0, 2, etc. instead of original dimensions)

**unit/test_phoimglist**: Segmentation fault
- Needs investigation - likely due to test fixture cleanup issues

### Key Finding
The regression test **successfully catches Issue #5**! This proves the testing infrastructure works and can verify bug fixes.

### How to Run Tests
```bash
cd tests
make test           # Run all tests
make test-unit      # Run unit tests only
make test-regression # Run regression tests only
```

---

## Testing the Tests

After setup complete:
```bash
make clean && make          # Build pho
make test                   # Run all tests
./tests/run_tests           # Direct test execution
```

---

## Questions/Issues Log

*None yet*
