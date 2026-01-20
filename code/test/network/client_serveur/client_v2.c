#include "syscall.h"
#include "nos_stdio.h"
#include "nos_stdlib.h"
#include "nos_string.h"

#define SERVER_ADDR 0
#define SERVER_PORT 8080
#define NUM_MESSAGES 5
#define BUFFER_SIZE 16384

char* msg = "Lorem ipsum dolor sit amet, consectetur adipiscing elit. Praesent eu cursus metus. Suspendisse scelerisque bibendum semper. Sed malesuada lobortis sem, nec iaculis metus maximus et. Proin porta, turpis quis vehicula sodales, mauris quam commodo augue, et laoreet ante metus et tellus. Praesent et risus eu leo tincidunt hendrerit cursus vel lectus. Praesent efficitur augue vel justo pretium tempor. Praesent eleifend nibh quam, tincidunt tristique nunc tempus quis. Sed sit amet ligula magna. Praesent ultrices magna nibh, sed dictum ante sollicitudin ac. In efficitur euismod eros, a auctor nulla vestibulum eu. Integer fringilla volutpat purus. Fusce elit sapien, volutpat vel consectetur et, vehicula eu massa. Sed dapibus est in dapibus porttitor \
Quisque malesuada diam ornare orci pretium, non condimentum purus pulvinar. Aenean in nunc a elit sagittis mollis. Etiam eget felis urna. Vestibulum varius nulla justo, non feugiat ex gravida nec. Orci varius natoque penatibus et magnis dis parturient montes, nascetur ridiculus mus. Donec pellentesque eros in vehicula viverra. Vivamus pharetra vulputate dapibus. Quisque euismod elementum felis rutrum efficitur. Nulla sit amet rutrum libero. Vivamus a purus ac eros interdum sodales. Fusce sem eros, mattis id quam commodo, viverra ultrices ligula. Suspendisse tempor justo sed dui gravida, eget bibendum lorem malesuada. In et tortor ut turpis lacinia blandit et eu erat. Nam orci purus, aliquet sit amet massa non, consequat finibus odio.\n"
"Fusce vitae dolor dui. Nunc semper ex nec metus interdum, luctus volutpat orci congue. Pellentesque sit amet tempor mauris, id euismod nisl. Pellentesque vitae purus pharetra, volutpat ex sit amet, malesuada risus. Nulla consectetur orci ut vehicula lacinia. Aenean interdum nulla mauris, at interdum nunc lacinia vel. Pellentesque habitant morbi tristique senectus et netus et malesuada fames ac turpis egestas. Aenean fringilla varius tellus, vitae rhoncus justo pretium eget. In ut erat sit amet tellus mollis tempus eu sed diam. Etiam fringilla dolor quis fringilla aliquam. Phasellus posuere risus ac nunc porta dictum. Donec eget velit tristique, venenatis est sed, rutrum magna.\n"
"Donec ac est eu nisl varius bibendum. Pellentesque eget commodo erat, sit amet convallis nibh. Proin consectetur id urna id laoreet. Sed condimentum lectus et dui gravida placerat non a risus. Cras ornare maximus leo. Aenean in condimentum nibh, a malesuada nulla. Phasellus eleifend sapien non metus iaculis, sit amet iaculis neque tincidunt.\n"
"Vivamus id nunc egestas libero sodales suscipit. Praesent justo turpis, accumsan non laoreet id, cursus ut sapien. Sed accumsan pharetra massa, eget porta neque aliquet eget. Vestibulum eu felis ornare, convallis metus ut, blandit velit. Proin commodo, leo at tincidunt porta, tellus neque cursus libero, a sodales ante dui et odio. Ut sed nisi vitae est tincidunt ornare non eu sem. In hac habitasse platea dictumst. Ut cursus neque eget turpis molestie iaculis."
"Duis eu lorem et justo finibus dapibus non id ex. Vestibulum ut placerat mauris. Morbi vulputate tellus ac diam \n"
"feugiat, a tristique odio tincidunt. Vivamus ultricies magna a ante mattis, porttitor interdum libero venenatis. Cras suscipit erat orci, eget ornare urna feugiat ut. Aliquam libero velit, rutrum vitae nulla at, mattis consectetur ex. Curabitur sodales fermentum tellus, eu fringilla lorem tristique vel.\n"

"Cras id lorem a sapien mollis ornare nec eu nunc. Proin mollis aliquam lobortis. Duis scelerisque aliquet mi, at molestie dolor blandit nec. Sed quam eros, finibus quis leo quis, malesuada posuere nisi. Proin eget orci interdum, dapibus odio sed, consectetur sem. Aliquam erat volutpat. Sed rhoncus aliquam orci quis facilisis.\n"

"Proin in felis eu sapien vulputate tristique. Donec posuere tellus et metus eleifend condimentum commodo a odio. Curabitur at quam mauris. Phasellus mattis facilisis diam eu aliquet. Vivamus ornare quis nisl at lacinia. Etiam et ante eleifend, sagittis nibh sit amet, sodales diam. Cras sed velit ac eros finibus porttitor vel sed risus. In auctor condimentum nisi, id pharetra augue aliquet semper. Nunc a dolor purus.\n"

"Integer vel nunc dignissim, tempus libero varius, egestas sem. Morbi tempor metus at viverra mollis. Donec a massa id odio varius bibendum. Etiam pharetra commodo lectus in cursus. Mauris ullamcorper pharetra dui vestibulum mattis. Donec viverra pharetra tincidunt. In egestas, est eget congue mattis, ante mauris auctor urna, et condimentum tellus lorem eu lorem. Donec ut risus vitae metus vulputate pharetra. Sed non dolor arcu. Proin tellus justo, consequat non tortor vitae, porttitor egestas velit. Nullam vel enim tempus, elementum libero id, pretium quam. Fusce quis lacus lacinia, auctor lorem vel, imperdiet enim.\n"

"Aliquam ut arcu odio. Integer hendrerit erat vitae orci tempus, non sollicitudin lorem accumsan. Vestibulum mollis vel turpis eget vestibulum. Sed vel aliquam ante. Curabitur auctor mauris dolor, vitae tristique libero ullamcorper non. Etiam placerat leo nibh, quis dapibus mi ultricies et. Donec eget dolor neque. Vestibulum vestibulum ligula ut aliquet cursus. Nulla sem nulla, suscipit ullamcorper tempor vel, mattis in sapien.\n"

"Curabitur vestibulum sapien in quam convallis venenatis. Vestibulum lacinia urna nibh, non lobortis sem fringilla non. Praesent sit amet quam scelerisque, bibendum sem eu, lacinia elit. Pellentesque ac quam nec mi bibendum imperdiet vel a nulla. Suspendisse sed convallis leo. Donec eget placerat tortor. Duis facilisis mollis orci ac semper. Donec non tristique est. Cras posuere risus sed risus convallis laoreet. Pellentesque convallis ante in nunc malesuada, sit amet posuere nisl sagittis. Curabitur tellus metus, imperdiet ut euismod non, viverra vel sem. Nunc a dui elementum, posuere nunc et, tincidunt metus. Morbi nec sem viverra, viverra mi et, tempor magna. Duis vel neque varius, malesuada risus sed, tempus nibh. Fusce libero massa, eleifend non feugiat nec, pellentesque id massa.\n"

"Maecenas placerat nisl et interdum aliquam. Donec tempus, massa sit amet sollicitudin blandit, nisl metus molestie nibh, ac eleifend quam tellus maximus turpis. Interdum et malesuada fames ac ante ipsum primis in faucibus. Mauris porttitor augue porttitor feugiat efficitur. Fusce placerat nibh quis augue laoreet, vel convallis lectus eleifend. Praesent quis cursus erat. Suspendisse sem arcu, auctor at mauris eget, semper suscipit ex. Vivamus eget nisi vel velit blandit dictum. Integer molestie ipsum et elit laoreet rhoncus. Ut accumsan gravida cursus. Nunc suscipit at arcu et vulputate.\n"

"Praesent ac interdum massa. Integer tempor enim vitae vestibulum blandit. Curabitur et odio porta, placerat erat quis, aliquet arcu. Nam sit amet purus placerat, vestibulum ligula vitae, porttitor massa. Proin consequat efficitur eros, vitae dignissim ligula blandit ac. In tristique nibh eget egestas dignissim. Etiam ac volutpat erat, vitae aliquam mi. Sed ut lacus libero. Orci varius natoque penatibus et magnis dis parturient montes, nascetur ridiculus mus. Morbi in ante feugiat, finibus lacus in, imperdiet libero. Nullam vestibulum laoreet mauris ac molestie. Nam lacinia gravida dui, ut aliquet velit laoreet ut. Sed sit amet euismod sem.\n"

"Cras ac varius magna, quis sodales nunc. Duis iaculis libero at efficitur tempor. Vestibulum efficitur et leo et rutrum. Nunc congue quam sed odio vulputate pulvinar. Nulla odio lacus, elementum in ipsum non, mattis vestibulum neque. Cras sed hendrerit metus. Duis nisl nisl, sollicitudin sit amet arcu in, gravida imperdiet sapien. Integer lorem diam, commodo eu libero id, auctor rutrum orci. Proin rhoncus suscipit sagittis. Mauris mollis quam nec nisl consectetur finibus. Praesent mattis nunc sit amet consequat varius. Mauris porta ipsum sed lobortis varius. Phasellus suscipit felis sed tortor facilisis, eget tincidunt turpis mattis. Suspendisse non mauris viverra, laoreet nunc et, euismod leo.\n"

"Phasellus at mauris vitae orci rutrum viverra non at sem. Ut tortor est, ullamcorper sed porta et, accumsan eget ex. Suspendisse sit amet quam tempus, lobortis ipsum ut, faucibus neque. Suspendisse mi libero, sollicitudin elementum porttitor vitae, suscipit eget quam. Quisque vitae urna vel felis rhoncus scelerisque. Phasellus a justo scelerisque, semper nisl et, pellentesque magna. In dictum sapien et nulla pulvinar ullamcorper. Quisque maximus id justo et auctor. Sed blandit urna in mi scelerisque commodo.\n"

"Vestibulum scelerisque non nibh non tincidunt. Etiam a sem non risus placerat ultrices ac eget justo. Proin vel viverra purus. Donec nec eros ultrices, hendrerit nulla in, interdum nisi. Vivamus at orci metus. Vestibulum vitae maximus orci, vel laoreet neque. Nunc faucibus ac ante semper tristique. In tristique diam ipsum, vel sodales velit hendrerit eu.\n"

"Nulla tempus vulputate nulla sed pretium. Suspendisse tempus, urna eleifend eleifend lobortis, sapien odio consequat erat, pharetra feugiat metus purus nec risus. Pellentesque varius sit amet sapien in mollis. Duis tincidunt, lectus nec placerat suscipit, quam nisl imperdiet odio, ut sollicitudin nulla enim vel sem. Sed sagittis eu tortor et lobortis. Integer mollis vulputate aliquam. Donec feugiat, dolor ac blandit eleifend, ligula mi malesuada nisi, eget rutrum tellus dui vitae turpis. Maecenas ultrices iaculis leo ut efficitur. Integer posuere dapibus mauris at semper. Donec ac mi et elit eleifend eleifend non in massa. Nulla facilisi. Maecenas tempor sodales velit ut facilisis. Curabitur faucibus placerat sem, eget consectetur metus consectetur at.\n"

"Donec sed orci at nisl placerat eleifend vitae eu dolor. Curabitur in laoreet dui. Nam lorem quam, iaculis pellentesque ante sit amet, tincidunt sollicitudin dolor. Nullam finibus faucibus velit, eu semper mauris accumsan quis. Nulla consequat tortor a sapien scelerisque consequat. Proin nec dapibus eros, ut tincidunt nunc. Nam non quam et ex tempor blandit non vel erat. Ut suscipit sit amet elit sit amet vestibulum eleifend. \n";

int main() {
    char buffer[BUFFER_SIZE];

    printf("========================================\n");
    printf("    Client Starting\n");
    printf("    Connecting to server %d:%d\n", SERVER_ADDR, SERVER_PORT);
    printf("========================================\n");

    int connId = connect(SERVER_ADDR, SERVER_PORT, 0);
    if (connId < 0) {
        printf("[Client] ERROR: connect() failed with %d\n", connId);
        return 1;
    }
    printf("[Client] Connected! (connId=%d)\n", connId);

    for (int i = 1; i <= NUM_MESSAGES; i++) {
        snprintf(msg, BUFFER_SIZE, "--- Message %d ---\n%s\n", i, msg);

        printf("[Client] Sending: '%s' (%d)\n", msg, strlen(msg));
        int sent = sendto(connId, msg, strlen(msg) + 1);

        if (sent < 0) {
            printf("[Client] ERROR: send failed with %d\n", sent);
            break;
        }

        int n = recvfrom(connId, buffer, BUFFER_SIZE - 1);
        if (n <= 0) {
            printf("[Client] ERROR: recv failed with %d\n", n);
            break;
        }
        buffer[n] = '\0';
        printf("[Client] Received: '%s'\n", buffer);

        Sleep(100000);
    }

    printf("[Client] Sending disconnect request...\n");
    strcpy(msg, "BYE");
    sendto(connId, msg, strlen(msg) + 1);

    int n = recvfrom(connId, buffer, BUFFER_SIZE - 1);
    if (n > 0) {
        buffer[n] = '\0';
        printf("[Client] Server response: '%s'\n", buffer);
    }

    close(connId);

    printf("========================================\n");
    printf("    Client Finished Successfully\n");
    printf("========================================\n");

    return 0;
}