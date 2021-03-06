#include "startscene.h"
#include "engine.h"

#include <QPropertyAnimation>
#include <QParallelAnimationGroup>
#include <QNetworkInterface>
#include <QGraphicsDropShadowEffect>

StartScene::StartScene()
{
    // game logo
    logo = new Pixmap("image/system/logo.png");
    logo->shift();
    logo->moveBy(0, -Config.Rect.height()/4);
    addItem(logo);

    //my e-mail address
    QFont email_font(Config.SmallFont);
    email_font.setStyle(QFont::StyleItalic);
    QGraphicsSimpleTextItem *email_text = addSimpleText("moligaloo@gmail.com", email_font);
    email_text->setBrush(Qt::white);
    email_text->setPos(Config.Rect.width()/2 - email_text->boundingRect().width(),
                       Config.Rect.height()/2 - email_text->boundingRect().height());

    server_log = NULL;
}

void StartScene::addButton(QAction *action){
    Button *button = new Button(action->text());
    button->setMute(false);

    connect(button, SIGNAL(clicked()), action, SLOT(trigger()));
    addItem(button);

    QRectF rect = button->boundingRect();
    int n = buttons.length();
    if(n < 5){
        button->setPos(- rect.width() - 5, (n - 1) * (rect.height() * 1.2));
    }else{
        button->setPos(5, (n - 6) * (rect.height() * 1.2));
    }

    buttons << button;
}

void StartScene::setServerLogBackground(){
    if(server_log){
        // make its background the same as background, looks transparent
        QPalette palette;
        palette.setBrush(QPalette::Base, backgroundBrush());
        server_log->setPalette(palette);
    }
}

extern irrklang::ISoundEngine *SoundEngine;

void StartScene::switchToServer(Server *server){
    if(SoundEngine){
        SoundEngine->drop();
        SoundEngine = NULL;
    }

    // performs leaving animation
    QPropertyAnimation *logo_shift = new QPropertyAnimation(logo, "pos");
    logo_shift->setEndValue(Config.Rect.topLeft());

    QPropertyAnimation *logo_shrink = new QPropertyAnimation(logo, "scale");
    logo_shrink->setEndValue(0.5);

    QParallelAnimationGroup *group = new QParallelAnimationGroup(this);
    group->addAnimation(logo_shift);
    group->addAnimation(logo_shrink);
    group->start(QAbstractAnimation::DeleteWhenStopped);

    foreach(Button *button, buttons)
        delete button;
    buttons.clear();

    server_log = new QTextEdit();
    server_log->setReadOnly(true);
    server_log->resize(700, 420);
    server_log->move(-400, -180);
    server_log->setFrameShape(QFrame::NoFrame);
    server_log->setFont(QFont("Verdana", 12));
    server_log->setTextColor(QColor("white"));
    setServerLogBackground();

    addWidget(server_log);

    printServerInfo();

    connect(server, SIGNAL(server_message(QString)), server_log, SLOT(append(QString)));

    update();
}

void StartScene::printServerInfo(){
    QStringList items;
    QList<QHostAddress> addresses = QNetworkInterface::allAddresses();
    foreach(QHostAddress address, addresses){
        quint32 ipv4 = address.toIPv4Address();
        if(ipv4)
            items << address.toString();
    }

    items.sort();

    foreach(QString item, items){
        if(item.startsWith("192.168.") || item.startsWith("10."))
            server_log->append(tr("Your LAN address: %1, this address is available only for hosts that in the same LAN").arg(item));
        else if(item == "127.0.0.1")
            server_log->append(tr("Your loopback address %1, this address is available only for your host").arg(item));
        else if(item.startsWith("5."))
            server_log->append(tr("Your Hamachi address: %1, the address is available for users that joined the same Hamachi network").arg(item));
        else if(!item.startsWith("169.254."))
            server_log->append(tr("Your other address: %1, if this is a public IP, that will be available for all cases").arg(item));
    }

    server_log->append(tr("Binding port number is %1").arg(Config.ServerPort));
    server_log->append(tr("Game mode is %1").arg(Sanguosha->getModeName(Config.GameMode)));
    server_log->append(tr("Player count is %1").arg(Sanguosha->getPlayerCount(Config.GameMode)));

    if(Config.OperationNoLimit)
        server_log->append(tr("There is no time limit"));
    else
        server_log->append(tr("Operation timeout is %1 seconds").arg(Config.OperationTimeout));    

    if(Config.ContestMode)
        server_log->append(tr("The contest mode is enabled"));

    server_log->append(tr("Free general choose is %1").arg(Config.FreeChoose ? tr("Enabled") : tr("Disabled")));
    server_log->append(tr("Secondary general is %1").arg(Config.Enable2ndGeneral ? tr("Enabled") : tr("Disabled")));

    if(Config.EnableAI)
        server_log->append(tr("This server is AI enabled, AI delay is %1 milliseconds").arg(Config.AIDelay));
    else
        server_log->append(tr("This server is AI disabled"));
}
