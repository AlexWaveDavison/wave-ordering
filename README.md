# Wave Ordering

A cross-platform (iOS, Android, desktop) wholesale ordering app built with
**Qt 6 / C++ / QML** — browse a product catalog, build a basket, and submit
orders.

This is a portfolio/demo project. The catalog ships pre-seeded with
**fictional sample data** (placeholder products, SKUs and prices).

## Status

This is a working scaffold:

- Catalog browsing with category filter + search
- Product detail view with quantity picker
- Basket with live totals, persisted across restarts
- "Place Order" -> saved to local order history with line items
- All data stored locally in SQLite (no live DB yet — see [Roadmap](#roadmap-to-a-live-database))

The catalog ships with ~48 sample products across 12 typical foodservice
wholesale categories (Cheese, Flour & Bakery, Pizza Toppings, Meat, Chicken,
Seafood, Potato Products, Packaging, Cleaning Materials, Accessories,
Beverages, Herbs & Spices), each with a placeholder SKU and price.

## Project layout

```
WaveOrdering/
├── CMakeLists.txt
├── src/                    C++ core (data layer, exposed to QML)
│   ├── database.{h,cpp}    SQLite schema + seed data
│   ├── categorymodel.*     Category list
│   ├── productmodel.*      Catalog list (filter/search)
│   ├── cartmodel.*         Basket (persisted)
│   ├── ordermodel.*         Order history
│   └── appcontroller.*     placeOrder() etc.
└── qml/
    ├── Main.qml            App window, tab navigation
    ├── Theme.qml           Colours, spacing, type scale (single source of truth)
    ├── pages/              CatalogPage, CartPage, OrdersPage
    └── components/         ProductCard, CartItemDelegate, dialogs, etc.
```

## Setup (Windows desktop — do this first)

Qt and the C++ toolchain aren't installed on this machine yet. Get the app
running on Windows desktop first; that's the fastest way to iterate on the UI
before touching iOS/Android.

1. **Install Qt 6.5+ (Qt Online Installer)**
   - Download from [qt.io/download-qt-installer](https://www.qt.io/download-qt-installer)
   - In the component selector, tick:
     - Qt 6.7 (or latest LTS) → **MSVC 2022 64-bit**
     - Qt 6.7 → **Qt Quick**, **Qt Quick Controls 2**, **Qt 5 Compatibility Module** (not required, but harmless)
     - Under "Developer and Designer Tools": **CMake**, **Ninja**, **Qt Creator**
   - Qt SQL's SQLite driver is built in — no extra download needed.

2. **Install MSVC Build Tools** (if not already present)
   - Visual Studio 2022 "Desktop development with C++" workload (Community edition is fine), **or** the standalone "Build Tools for Visual Studio 2022".

3. **Open the project**
   - Launch Qt Creator → Open File or Project → select `CMakeLists.txt` at the root of this repo.
   - Qt Creator will detect the Desktop (MSVC) kit. Configure the project.
   - Click **Run** (green play button). The app window opens at phone-ish proportions (412x915) so you can preview the mobile layout on desktop.

## iOS build (requires a Mac)

Apple requires Xcode on macOS to build/sign iOS apps — this can't be done from
Windows. Once you have Mac access:

1. Copy/clone this project to the Mac.
2. Install Xcode from the App Store, plus Qt's **iOS** component via the Qt
   Maintenance Tool (Qt 6.x → iOS).
3. In Qt Creator on the Mac, an **iOS** kit should appear automatically once Qt
   for iOS + Xcode are installed. Select it for this project.
4. Build & run on the iOS Simulator first (no Apple account needed).
5. To run on a physical iPhone or submit to the App Store, you'll need an
   **Apple Developer Program** membership ($99/year) for code signing —
   this is an Apple requirement for any app, regardless of framework.

A [GitHub Actions workflow](.github/workflows/ios-build.yml) is also included
that builds an **unsigned** `.ipa` on a free macOS runner (no Mac required),
which can then be sideloaded onto a device for testing.

## Android build

Can be done from Windows:

1. In the Qt Maintenance Tool, install the **Android** component for your Qt
   version.
2. In Qt Creator → Preferences → Devices → Android, let it auto-install/point
   to: JDK 17, Android SDK, Android NDK (Qt Creator has a one-click setup for
   this).
3. An **Android Qt 6.x** kit will appear. Select it for this project, then
   Run — it'll deploy to an emulator or a connected device over USB (with USB
   debugging enabled).

## Customizing the catalog

The sample catalog (`kCategories` / `kSeedProducts` in
[src/database.cpp](src/database.cpp)) is just placeholder data. To use your
own:

- Edit `kCategories` / `kSeedProducts` in
  [src/database.cpp](src/database.cpp) and delete the local SQLite database
  file so it reseeds (path printed via `QStandardPaths::AppDataLocation`,
  typically `%APPDATA%/Wave Ordering/Wave Ordering/waveordering.sqlite` on
  Windows) — **or** add an "Import CSV" feature for an in-app import flow.
- Add real prices to the `price` column — once non-zero, the UI shows
  "£X.XX"; a price of `0` shows "Price on application".

## Roadmap to a live database

The local schema (`categories`, `products`, `cart_items`, `orders`,
`order_items` — see [src/database.cpp](src/database.cpp)) is designed to map
directly onto a hosted backend later:

- **Likely path**: stand up Supabase (Postgres + REST/Realtime + Auth) with
  matching tables, then add a small `SyncService` (Qt Network) that:
  - pushes new orders to the backend on `placeOrder()`
  - pulls catalog/price updates into the local SQLite cache (so the app still
    works offline)
- This keeps the current offline-first UX working while adding multi-device
  sync, central order management, and (eventually) shared access for
  customers/staff.

## Next steps / ideas

- Product images (currently text-only cards)
- Bottom-nav icons (currently text labels)
- Admin/edit mode for prices and categories from within the app
- Order status updates (Submitted → Confirmed → Delivered) once there's a backend
- Authentication, once multiple users/devices are involved
