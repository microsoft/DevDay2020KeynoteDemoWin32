---
page_type: sample
languages:
- C++
products:
- Windows 10
description: "Simple demonstration of using GetContentRects in a Win32 application."
---

# Official Microsoft Sample

<!-- 
Guidelines on README format: https://review.docs.microsoft.com/help/onboard/admin/samples/concepts/readme-template?branch=master

Guidance on onboarding samples to docs.microsoft.com/samples: https://review.docs.microsoft.com/help/onboard/admin/samples/process/onboarding?branch=master

Taxonomies for products and languages: https://review.docs.microsoft.com/new-hope/information-architecture/metadata/taxonomies?branch=master
-->

This sample shows how to use the `GetContentRects` API to take advantage of dual-screen devices and to provide
the best experience on multi-display PCs. The sample uses a poly-filled version of the API based on [`EnumDisplayMonitors`](https://docs.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-enumdisplaymonitors) so you can test it on existing Desktop PCs before
Windows 10X is released.

## Contents


| File/folder       | Description                                |
|-------------------|--------------------------------------------|
| `src`             | Sample source code.                        |
| `.gitignore`      | Define what to ignore at commit time.      |
| `CHANGELOG.md`    | List of changes to the sample.             |
| `CONTRIBUTING.md` | Guidelines for contributing to the sample. |
| `README.md`       | This README file.                          |
| `LICENSE`         | The license for the sample.                |

## Prerequisites

This requires Visual Studio 2019 and a recent version of the Windows 10 Insider SDK. It does not require a 
Windows 10X device or the Windows 10X emulator, but they can be used as well.

## Setup

Clone the sample and then build the `DualScreenWin32` project. There is also an MSIX project for deploying the application to other devices (including the Windows 10X emulator). MSIX is the recommended way of packaging applications for Windows 10.

## Running the sample

By default the application draws a cyan rectangle with some information about the window's client area. 

If you have the Windows 10X
emulator (or a dual-screen device), you can snap the app across both screens to see the app reconfigure itself 
to draw one screen in cyan and the other in yellow. 

If you have a Desktop PC with multiple monitors, you can simply move the window between two monitors
to get the same effect as a dual-screen device. You might choose to update your existing Win32 apps even
if you don't specifically intend to target Windows 10X since you can improve the experience for your
existing Desktop users with multiple monitors.

If you have a single-screen Desktop, you can use the `Tools` -> `Toggle modes` menu item to emulate
different dual-screen configurations. Note that even on a single-screen device, if you move the window
partially off-screen, you will see the application responds and ensures the content remains on-screen.

## Key concepts

The key concept here is the use of the `GetContentRects` API to query the OS for the available ares where the application can draw. The app can still render content across the entire client area (spanning the gap on a 
dual-screen device, or spanning monitors on a Desktop), but being aware of the distinct content regions will enable you to optimize the experience.

## Contributing

This project welcomes contributions and suggestions.  Most contributions require you to agree to a
Contributor License Agreement (CLA) declaring that you have the right to, and actually do, grant us
the rights to use your contribution. For details, visit https://cla.opensource.microsoft.com.

When you submit a pull request, a CLA bot will automatically determine whether you need to provide
a CLA and decorate the PR appropriately (e.g., status check, comment). Simply follow the instructions
provided by the bot. You will only need to do this once across all repos using our CLA.

This project has adopted the [Microsoft Open Source Code of Conduct](https://opensource.microsoft.com/codeofconduct/).
For more information see the [Code of Conduct FAQ](https://opensource.microsoft.com/codeofconduct/faq/) or
contact [opencode@microsoft.com](mailto:opencode@microsoft.com) with any additional questions or comments.
