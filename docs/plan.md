# Pho Development Plan & Roadmap

**Document Version**: 2.0  
**Last Updated**: 2026-02-15  
**Status**: All security issues resolved - Ready for new features

---

## 📊 Current Status

### Security & Stability: ✅ COMPLETE

All 21 security issues and code quality improvements have been resolved:

| Category | Count | Status |
|----------|-------|--------|
| Critical | 6 | ✅ Fixed |
| High Priority | 6 | ✅ Fixed |
| Medium Priority | 4 | ✅ Fixed |
| Low Priority | 4 | ✅ Fixed |
| Verified No Issue | 1 | ✅ Closed |

**See `docs/completed-fixes.md` for detailed information on all resolved issues.**

### Test Suite: ✅ 50 TESTS PASSING

- **Unit Tests**: 5
- **Regression Tests**: 45
- **Coverage**: All security fixes verified

---

## 🎯 Future Roadmap

### Phase 1: GTK3 Migration (High Priority)

**Background**: Pho currently uses GTK+ 2.0 which is deprecated and no longer maintained. Migration to GTK3 is essential for long-term viability.

#### Tasks
1. **Research GTK3 API differences**
   - Identify deprecated functions
   - Map GTK2 → GTK3 replacements
   - Note behavioral changes

2. **Update build system**
   - Modify Makefile for GTK3
   - Update pkg-config calls
   - Handle macOS/Linux differences

3. **Migrate core UI components**
   - `gmain.c` - Main loop and event handling
   - `gwin.c` - Window management
   - `gdialogs.c` - Dialog boxes
   - `keydialog.c` - Keywords dialog

4. **Update drawing/rendering**
   - Replace `gdk_pixbuf_render_to_drawable`
   - Use Cairo for drawing operations
   - Handle HiDPI displays

5. **Test on multiple platforms**
   - macOS (primary development)
   - Linux (Debian/Ubuntu)
   - Verify all display modes work

**Estimated Effort**: 2-3 weeks  
**Risk**: High (complex API changes)

---

### Phase 2: Enhanced Image Formats (Medium Priority)

**Background**: Currently relies on GDK pixbuf loaders. Better format support would improve user experience.

#### Tasks
1. **WebP support**
   - Add WebP loading via libwebp
   - Preserve metadata

2. **HEIF/HEIC support**
   - Add HEIF loading for modern camera files
   - Handle orientation correctly

3. **SVG support**
   - Basic SVG rendering
   - Respect viewBox dimensions

4. **RAW format detection**
   - Identify RAW files even if can't display
   - Show appropriate error/warning

**Estimated Effort**: 1 week  
**Risk**: Low (well-documented libraries)

---

### Phase 3: Performance Improvements (Medium Priority)

#### Tasks
1. **Thumbnail caching**
   - Cache scaled thumbnails for large collections
   - Store in XDG_CACHE_HOME

2. **Async image loading**
   - Load images in background thread
   - Show spinner while loading
   - Maintain UI responsiveness

3. **Image prefetching**
   - Preload next/prev images
   - Configurable cache size

4. **Optimize rotation**
   - Use hardware acceleration where available
   - Cache rotated versions

**Estimated Effort**: 1-2 weeks  
**Risk**: Medium (threading complexity)

---

### Phase 4: New Features (Low Priority / Nice to Have)

#### 4.1 Slideshow Enhancements
- Transition effects (fade, slide)
- Configurable timing per image
- Random slide order option
- Pause on mouse movement

#### 4.2 EXIF Improvements
- Display more EXIF fields in info dialog
- Copy EXIF to rotated/modified images
- Strip EXIF option for privacy

#### 4.3 UI/UX Improvements
- Dark mode support
- Configurable keyboard shortcuts
- Mouse gesture support
- Touchscreen support (swipe navigation)

#### 4.4 Export Features
- Batch resize/export
- Export to different formats
- Create contact sheets

---

### Phase 5: Code Modernization (Ongoing)

#### Tasks
1. **Refactor global state**
   - Encapsulate in context structure
   - Reduce global variables
   - Improve testability

2. **Improve error handling**
   - Consistent error codes
   - Better error messages
   - User-facing error dialogs

3. **Code documentation**
   - Add Doxygen comments
   - Document public APIs
   - Architecture overview

4. **Static analysis integration**
   - Add clang-analyzer to CI
   - Address all warnings
   - Set up automated scanning

---

## 🔧 Maintenance Tasks

### Regular
- [ ] Update dependencies (GTK, libraries)
- [ ] Test on new OS versions
- [ ] Review and merge PRs
- [ ] Update documentation

### As Needed
- [ ] Security audits
- [ ] Performance profiling
- [ ] User feedback integration

---

## 📋 Decision Log

| Date | Decision | Rationale |
|------|----------|-----------|
| 2026-02-15 | Completed security fixes | All 21 issues resolved |
| 2026-02-15 | GTK3 migration priority | GTK2 is deprecated, blocking future development |
| 2026-02-15 | Keep C codebase | Rewrite in Rust/modern language would be too disruptive |

---

## 📝 Notes

### For Contributors
- Always run `make clean && make` before committing
- Add regression tests for bug fixes
- Update documentation for user-facing changes
- Follow existing code style (see AGENTS.md)

### For Maintainers
- Review security implications of new features
- Keep dependencies minimal
- Maintain backward compatibility where possible
- Test on both macOS and Linux before release

---

## 📚 Related Documents

- `docs/completed-fixes.md` - Historical record of security fixes
- `docs/lessons.md` - Lessons learned from development
- `docs/test-review.md` - Testing strategy and review
- `AGENTS.md` - Development guidelines and coding standards

---

*Last updated: 2026-02-15 - All security issues resolved, project ready for new features*
