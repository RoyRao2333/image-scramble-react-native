#import "NativeEncryptionCoreProvider.h"
#import <ReactCommon/CallInvoker.h>
#import <ReactCommon/TurboModule.h>
#import "NativeEncryptionCore.h"

@implementation NativeEncryptionCoreProvider

- (std::shared_ptr<facebook::react::TurboModule>)getTurboModule:
    (const facebook::react::ObjCTurboModule::InitParams &)params {
    return std::make_shared<facebook::react::NativeEncryptionCore>(
        params.jsInvoker);
}

@end
